# Simple DLL Injector

Este projeto é um injetor de DLL escrito em C++ utilizado como prova de conceito. Ele permite que os usuários injetem uma DLL em um processo em execução no Windows utilizando LoadLibrary.

## Funcionalidade

Funciona localizando o processo alvo, alocando memória dentro desse processo para o caminho da DLL, escrevendo o caminho da DLL nessa memória alocada e, finalmente, criando uma nova thread no processo alvo que carrega a DLL usando a função `LoadLibraryA`.

## Como usar

Para usar o injetor de DLL, execute o seguinte comando na linha de comando:

```
simple-dll-injector.exe <path_to_dll> <process_name>
```

Exemplo:

```
simple-dll-injector.exe myDll.dll targetProcess.exe
```