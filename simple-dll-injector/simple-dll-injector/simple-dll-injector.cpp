#include <iostream>
#include <Windows.h>

// Function to inject DLL into the target process
bool InjectDLL(DWORD processId, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        std::cout << "Failed to open the target process." << std::endl;
        return false;
    }

    LPVOID addr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    if (addr == NULL) {
        std::cout << "Failed to get the address of LoadLibraryA function." << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    LPVOID arg = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(dllPath), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (arg == NULL) {
        std::cout << "Failed to allocate memory in the target process." << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, arg, dllPath, strlen(dllPath), NULL)) {
        std::cout << "Failed to write DLL path into the target process." << std::endl;
        VirtualFreeEx(hProcess, arg, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, 0, NULL);
    if (hThread == NULL) {
        std::cout << "Failed to create a remote thread in the target process." << std::endl;
        VirtualFreeEx(hProcess, arg, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, arg, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <process_id> <dll_path>" << std::endl;
        return 1;
    }

    DWORD processId = std::stoi(argv[1]);
    const char* dllPath = argv[2];

    if (InjectDLL(processId, dllPath)) {
        std::cout << "DLL injection successful!" << std::endl;
    }
    else {
        std::cout << "DLL injection failed." << std::endl;
    }

    return 0;
}
