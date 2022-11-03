/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Ntdll.h

Abstract:

    This Module implements the ntdll helper procedures.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SYSTEM_NTDLL_H_
#define _WIN32EX_SYSTEM_NTDLL_H_

#include "../Internal/version.h"
#define WIN32EX_SYSTEM_NTDLL_H_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SYSTEM_NTDLL_H_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SYSTEM_NTDLL_H_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "../Internal/misc.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#ifndef STATUS_ENTRYPOINT_NOT_FOUND
#define STATUS_ENTRYPOINT_NOT_FOUND ((DWORD)0xC0000139L)
#endif

////
//// 미리 USE_NTDLL_STATIC_LINK가 정의되있지 않았다면,
//// Visual Studio 2010 이상일때는 ntdll.lib를 기본으로 정적 링크하도록 합니다.
////
//
// #if !defined(USE_NTDLL_STATIC_LINK) && defined(_MSC_VER) && (_MSC_VER > 1600)
// #define USE_NTDLL_STATIC_LINK
// #endif

//
//  Native API를 사용할때, 자주 사용하는 매크로.
//

//
// This works "generically" for Unicode and Ansi/Oem strings.
// Usage:
//   const static UNICODE_STRING FooU = RTL_CONSTANT_STRING(L"Foo");
//   const static         STRING Foo  = RTL_CONSTANT_STRING( "Foo");
// instead of the slower:
//   UNICODE_STRING FooU;
//           STRING Foo;
//   RtlInitUnicodeString(&FooU, L"Foo");
//          RtlInitString(&Foo ,  "Foo");
//
// Or:
//   const static char szFoo[] = "Foo";
//   const static STRING sFoo = RTL_CONSTANT_STRING(szFoo);
//
// This will compile without error or warning in C++. C will get a warning.
//
#ifdef __cplusplus
extern "C++"
{
    char _RTL_CONSTANT_STRING_type_check(const char *s);
    char _RTL_CONSTANT_STRING_type_check(const WCHAR *s);
    // __typeof would be desirable here instead of sizeof.
    template <size_t N> class _RTL_CONSTANT_STRING_remove_const_template_class;
    template <> class _RTL_CONSTANT_STRING_remove_const_template_class<sizeof(char)>
    {
      public:
        typedef char T;
    };
    template <> class _RTL_CONSTANT_STRING_remove_const_template_class<sizeof(WCHAR)>
    {
      public:
        typedef WCHAR T;
    };
#define _RTL_CONSTANT_STRING_remove_const_macro(s)                                                                     \
    (const_cast<_RTL_CONSTANT_STRING_remove_const_template_class<sizeof((s)[0])>::T *>(s))
}
#else
char _RTL_CONSTANT_STRING_type_check(const void *s);
#define _RTL_CONSTANT_STRING_remove_const_macro(s) (s)
#endif
#define RTL_CONSTANT_STRING(s)                                                                                         \
    {                                                                                                                  \
        sizeof(s) - sizeof((s)[0]), sizeof(s) / sizeof(_RTL_CONSTANT_STRING_type_check(s)),                            \
            _RTL_CONSTANT_STRING_remove_const_macro(s)                                                                 \
    }

//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

//
// Generic test for information on any status value.
//

#ifndef NT_INFORMATION
#define NT_INFORMATION(Status) ((((ULONG)(Status)) >> 30) == 1)
#endif

//
// Generic test for warning on any status value.
//

#ifndef NT_WARNING
#define NT_WARNING(Status) ((((ULONG)(Status)) >> 30) == 2)
#endif

//
// Generic test for error on any status value.
//

#ifndef NT_ERROR
#define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)
#endif

//
//  ntdef.h가 정의되어있지 않다면 ntdef.h의 내용을 정의합니다,.
//

#if (!defined(_NTDEF_)) && (!defined(_NTSECAPI_))

//
//  ntdef.h의 내용을 정의하십시오.
// 보통 WDK에 포함되어있으며, 이 헤더파일은 다양한 환경을 지원하는것을 목표로 하기때문에
// ntdef.h를 직접 참조하지는 않았습니다. 그렇기 때문에 해당 내용을 직접 넣어주십시오.
//

typedef LONG NTSTATUS, *PNTSTATUS;

#ifndef _WINTERNL_
typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING, *PSTRING;

typedef struct _OBJECT_ATTRIBUTES
{
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;       // Points to type SECURITY_DESCRIPTOR
    PVOID SecurityQualityOfService; // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
#endif // _WINTERNL_

//
//  ntdef.h의 내용이 정의되었으므로, _NTDEF_를 정의하여 재정의 문제를 예방합니다.
//

#ifndef _NTDEF_
#define _NTDEF_
///
/// ntdef.h
///
#define OBJ_INHERIT 0x00000002L
#define OBJ_PERMANENT 0x00000010L
#define OBJ_EXCLUSIVE 0x00000020L
#define OBJ_CASE_INSENSITIVE 0x00000040L
#define OBJ_OPENIF 0x00000080L
#define OBJ_OPENLINK 0x00000100L
#define OBJ_KERNEL_HANDLE 0x00000200L
#define OBJ_FORCE_ACCESS_CHECK 0x00000400L
#define OBJ_IGNORE_IMPERSONATED_DEVICEMAP 0x00000800L
#define OBJ_DONT_REPARSE 0x00001000L
#define OBJ_VALID_ATTRIBUTES 0x00001FF2L
#endif // _NTDEF_
#endif // (!defined(_NTDEF_)) && (!defined(_NTSECAPI_))

//
//  winternal.h가 정의되어있지 않다면 winternal.h의 내용을 정의합니다,.
//

#ifndef _WINTERNL_
//
//  Native API에서 사용하는 구조체, 열거형 상수, 매크로 등을 정의합니다.
//
// 이때, winternal.h에 이미 정의되어있는 내용은 정의하지 않도록 처리합니다.
//
typedef enum _OBJECT_INFORMATION_CLASS
{
    ObjectBasicInformation,
    ObjectTypeInformation
} OBJECT_INFORMATION_CLASS;

//
//  Public Object Information definitions
//

typedef struct _PUBLIC_OBJECT_BASIC_INFORMATION
{
    ULONG Attributes;
    ACCESS_MASK GrantedAccess;
    ULONG HandleCount;
    ULONG PointerCount;

    ULONG Reserved[10]; // reserved for internal use

} PUBLIC_OBJECT_BASIC_INFORMATION, *PPUBLIC_OBJECT_BASIC_INFORMATION;

typedef struct __PUBLIC_OBJECT_TYPE_INFORMATION
{

    UNICODE_STRING TypeName;

    ULONG Reserved[22]; // reserved for internal use

} PUBLIC_OBJECT_TYPE_INFORMATION, *PPUBLIC_OBJECT_TYPE_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation = 0,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemInterruptInformation = 23,
    SystemExceptionInformation = 33,
    SystemRegistryQuotaInformation = 37,
    SystemLookasideInformation = 45,
    SystemFullProcessInformation = 148
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_PROCESS_INFORMATION
{
    ULONG NextEntryOffset;
    BYTE Reserved1[52];
    UNICODE_STRING ImageName;
    LONG BasePriority;
    HANDLE UniqueProcessId;
    PVOID Reserved3;
    ULONG HandleCount;
    BYTE Reserved4[4];
    PVOID Reserved5[11];
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER Reserved6[6];
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

#define USE_UNDOCUMENTED_SYSTEM_INFORMATION_CLASS
#endif

//
//  정적/동적 링크와 사용 언어(C, C++)를 고려하여 API 원형을 선언합니다.
//

#ifdef USE_NTDLL_STATIC_LINK
#pragma comment(lib, "ntdll.lib")

//
//  Native API prototypes.
//

EXTERN_C_START
ULONG
NTAPI
RtlNtStatusToDosError(_In_ NTSTATUS Status);

__kernel_entry NTSTATUS NTAPI NtMakePermanentObject(_In_ HANDLE Handle);

__kernel_entry NTSTATUS NTAPI NtMakeTemporaryObject(_In_ HANDLE Handle);

__kernel_entry NTSTATUS NTAPI NtQueryObject(_In_opt_ HANDLE Handle,
                                            _In_ OBJECT_INFORMATION_CLASS ObjectInformationClass,
                                            _Out_opt_ PVOID ObjectInformation, _In_ ULONG ObjectInformationLength,
                                            _Out_opt_ PULONG ReturnLength);

__kernel_entry NTSTATUS NTAPI NtQuerySystemInformation(_In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                                       _Out_ PVOID SystemInformation,
                                                       _In_ ULONG SystemInformationLength,
                                                       _Out_opt_ PULONG ReturnLength);
EXTERN_C_END
#else
#ifdef __cplusplus
static ULONG(NTAPI *_RtlNtStatusToDosError)(NTSTATUS Status) = (ULONG(NTAPI *)(NTSTATUS))
    GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlNtStatusToDosError");
static NTSTATUS(NTAPI *_NtMakeTemporaryObject)(HANDLE Handle) = (NTSTATUS(NTAPI *)(HANDLE))
    GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtMakeTemporaryObject");
static NTSTATUS(NTAPI *_NtMakePermanentObject)(HANDLE Handle) = (NTSTATUS(NTAPI *)(HANDLE))
    GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtMakePermanentObject");
static NTSTATUS(NTAPI *_NtQueryObject)(HANDLE Handle, OBJECT_INFORMATION_CLASS ObjectInformationClass,
                                       PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength) =
    (NTSTATUS(NTAPI *)(HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG,
                       PULONG))GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryObject");

static NTSTATUS(NTAPI *_NtQuerySystemInformation)(SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                                  PVOID SystemInformation, ULONG SystemInformationLength,
                                                  PULONG ReturnLength) =
    (NTSTATUS(NTAPI *)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG))GetProcAddress(GetModuleHandleA("ntdll.dll"),
                                                                                      "NtQuerySystemInformation");
#else
static ULONG(NTAPI *_RtlNtStatusToDosError)(NTSTATUS Status) = NULL;
static NTSTATUS(NTAPI *_NtMakeTemporaryObject)(HANDLE Handle) = NULL;
static NTSTATUS(NTAPI *_NtMakePermanentObject)(HANDLE Handle) = NULL;
static NTSTATUS(NTAPI *_NtQueryObject)(HANDLE Handle, OBJECT_INFORMATION_CLASS ObjectInformationClass,
                                       PVOID ObjectInformation, ULONG ObjectInformationLength,
                                       PULONG ReturnLength) = NULL;
static NTSTATUS(NTAPI *_NtQuerySystemInformation)(SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                                  PVOID SystemInformation, ULONG SystemInformationLength,
                                                  PULONG ReturnLength) = NULL;
#endif

WIN32EX_ALWAYS_INLINE ULONG NTAPI RtlNtStatusToDosError(_In_ NTSTATUS Status)
{
    HMODULE hModule;
    if (_RtlNtStatusToDosError)
    {
        return _RtlNtStatusToDosError(Status);
    }

    hModule = GetModuleHandleA("ntdll.dll");
    if (!hModule)
        return ERROR_NOT_FOUND;

    _RtlNtStatusToDosError = (ULONG(NTAPI *)(NTSTATUS))GetProcAddress(hModule, "RtlNtStatusToDosError");

    return (_RtlNtStatusToDosError ? _RtlNtStatusToDosError(Status) : ERROR_NOT_FOUND);
}

WIN32EX_ALWAYS_INLINE NTSTATUS NTAPI NtMakeTemporaryObject(_In_ HANDLE Handle)
{
    HMODULE hModule;
    if (_NtMakeTemporaryObject)
    {
        return _NtMakeTemporaryObject(Handle);
    }

    hModule = GetModuleHandleA("ntdll.dll");
    if (!hModule)
        return STATUS_ENTRYPOINT_NOT_FOUND;

    _NtMakeTemporaryObject = (NTSTATUS(NTAPI *)(HANDLE))GetProcAddress(hModule, "NtMakeTemporaryObject");

    return (_NtMakeTemporaryObject ? _NtMakeTemporaryObject(Handle) : STATUS_ENTRYPOINT_NOT_FOUND);
}

WIN32EX_ALWAYS_INLINE NTSTATUS NTAPI NtMakePermanentObject(_In_ HANDLE Handle)
{
    HMODULE hModule;
    if (_NtMakePermanentObject)
    {
        return _NtMakePermanentObject(Handle);
    }

    hModule = GetModuleHandleA("ntdll.dll");
    if (!hModule)
        return STATUS_ENTRYPOINT_NOT_FOUND;

    _NtMakePermanentObject = (NTSTATUS(NTAPI *)(HANDLE))GetProcAddress(hModule, "NtMakePermanentObject");

    return (_NtMakePermanentObject ? _NtMakePermanentObject(Handle) : STATUS_ENTRYPOINT_NOT_FOUND);
}

WIN32EX_ALWAYS_INLINE NTSTATUS NTAPI NtQueryObject(_In_opt_ HANDLE Handle,
                                                   _In_ OBJECT_INFORMATION_CLASS ObjectInformationClass,
                                                   _Out_opt_ PVOID ObjectInformation,
                                                   _In_ ULONG ObjectInformationLength, _Out_opt_ PULONG ReturnLength)
{
    HMODULE hModule;
    if (_NtQueryObject)
    {
        return _NtQueryObject(Handle, ObjectInformationClass, ObjectInformation, ObjectInformationLength, ReturnLength);
    }

    hModule = GetModuleHandleA("ntdll.dll");
    if (!hModule)
        return STATUS_ENTRYPOINT_NOT_FOUND;

    _NtQueryObject = (NTSTATUS(NTAPI *)(HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG, PULONG))GetProcAddress(
        GetModuleHandleA("ntdll.dll"), "NtQueryObject");

    return (_NtQueryObject ? _NtQueryObject(Handle, ObjectInformationClass, ObjectInformation, ObjectInformationLength,
                                            ReturnLength)
                           : STATUS_ENTRYPOINT_NOT_FOUND);
}

WIN32EX_ALWAYS_INLINE NTSTATUS NTAPI NtQuerySystemInformation(_In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                                              _Out_ PVOID SystemInformation,
                                                              _In_ ULONG SystemInformationLength,
                                                              _Out_opt_ PULONG ReturnLength)
{
    HMODULE hModule;
    if (_NtQuerySystemInformation)
    {
        return _NtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength,
                                         ReturnLength);
    }

    hModule = GetModuleHandleA("ntdll.dll");
    if (!hModule)
        return STATUS_ENTRYPOINT_NOT_FOUND;

    _NtQuerySystemInformation =
        (NTSTATUS(NTAPI *)(SYSTEM_INFORMATION_CLASS, _Out_ PVOID, ULONG, _Out_opt_ PULONG))GetProcAddress(
            hModule, "NtQuerySystemInformation");

    return (_NtQuerySystemInformation ? _NtQuerySystemInformation(SystemInformationClass, SystemInformation,
                                                                  SystemInformationLength, ReturnLength)
                                      : STATUS_ENTRYPOINT_NOT_FOUND);
}
#endif

#endif // _WIN32EX_SYSTEM_NTDLL_H_
