#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD fwdReason, LPVOID lpvReserved) {
  // Handle to DLL module
  // Reason for calling the function (DLL opened? Basically a call event)
  // Reserved -- Has to be there, just a random spot basically.

  switch(fwdReason) {
    case DLL_PROCESS_ATTACH:
      //initialize once for each new process, so on start.
      MessageBoxW(NULL, L"DLL Injected", L"=^..^=", MB_ICONQUESTION | MB_OK);
      break;
    case DLL_PROCESS_DETACH:
      MessageBoxW(NULL, L"See you next time", L"=^..^=", MB_ICONINFORMATION | MB_OK);
  }

  return TRUE;
}