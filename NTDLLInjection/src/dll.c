#include <windows.h>

DWORD WINAPI MyThreadFunction(LPVOID lpParam) {
    // Show a message box
    MessageBoxW(NULL, L"DIEEEEE!!!!!!", L"(⁎˃ᆺ˂)", MB_ICONQUESTION | MB_OK);

    // Clean up and exit the thread
    ExitThread(0);
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD fwdReason, LPVOID lpvReserved) {
  // Handle to DLL module
  // Reason for calling the function (DLL opened? Basically a call event)
  // Reserved -- Has to be there, just a random spot basically.

  switch(fwdReason) {
    case DLL_PROCESS_ATTACH:
      //initialize once for each new process, so on start.
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MyThreadFunction, NULL, 0, NULL);
      break;
    case DLL_PROCESS_DETACH:
      break;
  }

  return TRUE;
}