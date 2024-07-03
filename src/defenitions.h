#pragma once
#include <windows.h>
#include <stdio.h>

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define okay(msg, ...) printf("[+] " msg "\n", ##__VA_ARGS__)
#define info(msg, ...) printf("[i] " msg "\n", ##__VA_ARGS__)
#define warn(msg, ...) printf("[-] " msg "\n", ##__VA_ARGS__)

/*------------ STRUCTURES ------------*/
/* Due to how NT works, we're forced to reverse engineer. Therefore, some structs we need to reference need to be implemented manually*/
/* using https://www.vergiliusproject.com/ (Vergilius Project), we can find the structures we need and define them via the code provided here.*/

//0x30 bytes (sizeof)
typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;                                                           //0x0
    VOID* RootDirectory;                                                    //0x8
    struct _UNICODE_STRING* ObjectName;                                     //0x10
    ULONG Attributes;                                                       //0x18
    VOID* SecurityDescriptor;                                               //0x20
    VOID* SecurityQualityOfService;                                         //0x28
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES; 

//0x10 bytes (sizeof)
typedef struct _CLIENT_ID {
    VOID* UniqueProcess;                                                    //0x0
    VOID* UniqueThread;                                                     //0x8
} CLIENT_ID, *PCLIENT_ID; 

typedef struct _PS_ATTRIBUTE {
    ULONG_PTR Attribute;
    SIZE_T Size;
    union
    {
        ULONG_PTR Value;
        PVOID ValuePtr;
    };
    PSIZE_T ReturnLength;
} PS_ATTRIBUTE, *PPS_ATTRIBUTE;

typedef struct _PS_ATTRIBUTE_LIST {
    SIZE_T TotalLength;
    PS_ATTRIBUTE Attributes[1];
} PS_ATTRIBUTE_LIST, *PPS_ATTRIBUTE_LIST;

/*------------ FUNCTION PROTOTYPES ------------*/
/* We also need to define the functions we are trying to call manually for NT, we can find them via https://ntdoc.m417z.com/ (NTDOC)*/

typedef NTSTATUS (NTAPI* NtOpenProcess) (
    _Out_ PHANDLE ProcessHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_opt_ PCLIENT_ID ClientId
);

typedef NTSTATUS (NTAPI* NtCreateThreadEx) (
    _Out_ PHANDLE ThreadHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_ HANDLE ProcessHandle,
    _In_ PVOID StartRoutine,
    _In_opt_ PVOID Argument,
    _In_ ULONG CreateFlags, // THREAD_CREATE_FLAGS_*
    _In_ SIZE_T ZeroBits,
    _In_ SIZE_T StackSize,
    _In_ SIZE_T MaximumStackSize,
    _In_opt_ PPS_ATTRIBUTE_LIST AttributeList
);

typedef NTSTATUS (NTAPI* NtClose) (
    _In_ HANDLE Handle
);

typedef NTSTATUS (NTAPI* NtAllocateVirtualMemoryEx) (
    _In_ HANDLE ProcessHandle,
    _Inout_ _At_(*BaseAddress, _Readable_bytes_(*RegionSize) _Writable_bytes_(*RegionSize) _Post_readable_byte_size_(*RegionSize)) PVOID *BaseAddress,
    _Inout_ PSIZE_T RegionSize,
    _In_ ULONG AllocationType,
    _In_ ULONG PageProtection,
    _Inout_updates_opt_(ExtendedParameterCount) PMEM_EXTENDED_PARAMETER ExtendedParameters,
    _In_ ULONG ExtendedParameterCount
);


typedef NTSTATUS (NTAPI* NtWriteVirtualMemory) (
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID BaseAddress,
    _In_reads_bytes_(BufferSize) PVOID Buffer,
    _In_ SIZE_T BufferSize,
    _Out_opt_ PSIZE_T NumberOfBytesWritten
);
