#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

HANDLE FindProc(const char procName[260]) {
    printf("Creating snapshot of all running processes...\n");
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (!hSnap) {
        printf("Error creating snapshot: %lu\n", GetLastError());
        return nullptr;
    }

    PROCESSENTRY32 peEntry;
    peEntry.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnap, &peEntry)) {
        printf("Error retrieving process info: %lu\n", GetLastError());
        CloseHandle(hSnap);
        return nullptr;
    }

    do {
        if (!_stricmp(peEntry.szExeFile, procName)) {
            printf("Match found with PID: %u\n", peEntry.th32ProcessID);
            CloseHandle(hSnap);
            HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, peEntry.th32ProcessID);
            if (!proc) {
                printf("Error opening process with PID: %u, error code: %lu\n", peEntry.th32ProcessID, GetLastError());
                return nullptr;
            }
            return proc;
        }
    } while (Process32Next(hSnap, &peEntry));

    printf("No matching process found.\n");
    CloseHandle(hSnap);
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <path_to_dll> <process_name>\n", argv[0]);
        return 1;
    }

    const char* partialPath = argv[1];
    const char* procName = argv[2];

    char dllPath[MAX_PATH];
    GetFullPathName(partialPath, MAX_PATH, dllPath, NULL);

    if (GetFileAttributes(dllPath) == INVALID_FILE_ATTRIBUTES) {
        printf("DLL file does not exist: %s, error code: %lu\n", dllPath, GetLastError());
        return 1;
    }

    printf("Full DLL Path: %s\n", dllPath);
    printf("Process name: %s\n", procName);

    printf("Searching for the process...\n");
    HANDLE hProc = FindProc(procName);

    if (!hProc || hProc == INVALID_HANDLE_VALUE) {
        printf("Error finding process, error code: %lu\n", GetLastError());
        return 1;
    }

    printf("Allocating memory in the target process...\n");
    LPVOID loc = VirtualAllocEx(hProc, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!loc) {
        printf("Error in memory allocation, error code: %lu\n", GetLastError());
        return 1;
    }

    printf("Writing DLL path into the target process memory...\n");
    if (!WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, NULL)) {
        printf("Error in WriteProcessMemory, error code: %lu\n", GetLastError());
        VirtualFreeEx(hProc, loc, strlen(dllPath) + 1, MEM_FREE);
        return 1;
    }

    printf("Creating remote thread to load the DLL...\n");
    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, NULL);

    if (!hThread) {
        printf("Error in CreateRemoteThread, error code: %lu\n", GetLastError()); 
        return 1;
    }
    Beep(1000, 500);
    printf("DLL successfully injected!\n");
    CloseHandle(hProc);
    CloseHandle(hThread);
    return 0;
}
