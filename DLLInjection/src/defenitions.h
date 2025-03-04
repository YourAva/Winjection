#include <windows.h>
#include <stdio.h>

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define okay(msg, ...) printf("[+] " msg "\n", ##__VA_ARGS__)
#define info(msg, ...) printf("[i] " msg "\n", ##__VA_ARGS__)
#define warn(msg, ...) printf("[-] " msg "\n", ##__VA_ARGS__)