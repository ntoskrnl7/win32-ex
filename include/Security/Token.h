/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Token.h

Abstract:

    This Module implements the token procedures.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <Windows.h>

#include <Psapi.h>
#pragma comment(lib, "psapi.lib")

#include "Sid.h"

#ifdef __cplusplus
#include <functional>
#if defined(_MSC_VER) && _MSC_VER < 1600
namespace std
{
using std::tr1::function;
} // namespace std
#endif
#endif // __cplusplus

#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8 0x0602
#endif

#if (_WIN32_WINNT < _WIN32_WINNT_WIN8)
FORCEINLINE
HANDLE
GetCurrentProcessToken(VOID)
{
    return (HANDLE)(LONG_PTR)-4;
}
#endif

FORCEINLINE PVOID GetTokenInfo(_In_ HANDLE TokenHandle, _In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
                               _Out_opt_ PDWORD ReturnLength)
{
    DWORD returnedLength = 0;
    DWORD bufferSize = 128;
    PVOID buffer = HeapAlloc(GetProcessHeap(), 0, bufferSize);
    if (!buffer)
    {
        return NULL;
    }

retry:
    if (GetTokenInformation(TokenHandle, TokenInformationClass, buffer, bufferSize, &returnedLength))
    {
        if (ReturnLength)
            *ReturnLength = returnedLength;
        return buffer;
    }
    else
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            PVOID newBuffer = HeapReAlloc(GetProcessHeap(), 0, buffer, returnedLength);
            if (!newBuffer)
            {
                HeapFree(GetProcessHeap(), 0, buffer);
                return NULL;
            }
            buffer = newBuffer;
            bufferSize = returnedLength;
            goto retry;
        }
    }

    HeapFree(GetProcessHeap(), 0, buffer);
    return NULL;
}

FORCEINLINE VOID FreeTokenInfo(_In_ PVOID Info)
{
    HeapFree(GetProcessHeap(), 0, Info);
}

FORCEINLINE PTOKEN_USER GetTokenUser(_In_ HANDLE TokenHandle)
{
    return (PTOKEN_USER)GetTokenInfo(TokenHandle, TokenUser, NULL);
}

FORCEINLINE VOID FreeTokenUser(_In_ PTOKEN_USER TokenUser)
{
    HeapFree(GetProcessHeap(), 0, TokenUser);
}

FORCEINLINE PTOKEN_GROUPS GetTokenGroups(_In_ HANDLE TokenHandle)
{
    return (PTOKEN_GROUPS)GetTokenInfo(TokenHandle, TokenGroups, NULL);
}

FORCEINLINE VOID FreeTokenGroups(_In_ PTOKEN_GROUPS TokenGroups)
{
    HeapFree(GetProcessHeap(), 0, TokenGroups);
}

FORCEINLINE VOID FreeTokenUserSid(_In_ PSID Sid)
{
    HeapFree(GetProcessHeap(), 0, Sid);
}

FORCEINLINE PSID DuplicateTokenUserSid(_In_ PSID Sid, _In_ DWORD SidLength)
{
    PVOID sid = HeapAlloc(GetProcessHeap(), 0, SidLength);
    if (!sid)
        return NULL;

    if (!CopySid(SidLength, sid, Sid))
    {
        HeapFree(GetProcessHeap(), 0, sid);
        return NULL;
    }
    return sid;
}

FORCEINLINE PSID GetProcessTokenUserSid(_In_ HANDLE hProcess)
{
    HANDLE TokenHandle;
    PSID sid = NULL;

    if (OpenProcessToken(hProcess, MAXIMUM_ALLOWED, &TokenHandle))
    {
        PTOKEN_USER user = GetTokenUser(TokenHandle);
        CloseHandle(TokenHandle);
        if (user)
            return DuplicateTokenUserSid(user->User.Sid, GetLengthSid(user->User.Sid));
    }

    return NULL;
}

/// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership
#ifdef __cplusplus
FORCEINLINE BOOL IsUserAdmin(HANDLE hToken = NULL)
#else
FORCEINLINE BOOL IsUserAdmin(HANDLE hToken)
#endif
/*++
Routine Description: This routine returns TRUE if the caller's
process is a member of the Administrators local group. Caller is NOT
expected to be impersonating anyone and is expected to be able to
open its own process and process token.
Arguments: None.
Return Value:
   TRUE - Caller has Administrators local group.
   FALSE - Caller does not have Administrators local group. --
*/
{
    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    b = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0,
                                 0, &AdministratorsGroup);
    if (b)
    {
        if (!CheckTokenMembership(hToken, AdministratorsGroup, &b))
        {
            b = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    return (b);
}

FORCEINLINE BOOL EqualTokenUserSid(_In_ HANDLE TokenHandle, _In_ PSID Sid)
{
    BOOL equal = FALSE;
    DWORD returnedLength = 0;
    PTOKEN_USER user = (PTOKEN_USER)GetTokenInfo(TokenHandle, TokenUser, &returnedLength);
    if (!user)
        return FALSE;

    equal = EqualSid(user->User.Sid, Sid);
    FreeTokenInfo(user);
    return equal;
}

FORCEINLINE BOOL IsNetworkServiceToken(_In_ HANDLE TokenHandle)
{
    return EqualTokenUserSid(TokenHandle, &NetworkServiceSid);
}

FORCEINLINE BOOL IsLocalServiceToken(_In_ HANDLE TokenHandle)
{
    return EqualTokenUserSid(TokenHandle, &LocalServiceSid);
}

FORCEINLINE BOOL IsLocalSystemToken(_In_ HANDLE TokenHandle)
{
    return EqualTokenUserSid(TokenHandle, &LocalSystemSid);
}

#ifdef __cplusplus
FORCEINLINE HANDLE LookupTokenEx2(_In_ DWORD DesireAccess,
                                  _In_ std::function<BOOL(DWORD, HANDLE, PVOID)> TokenCondition,
                                  _Inout_opt_ PVOID Context)
#else
typedef BOOL (*PTOKEN_CONDITION_ROUTINE_EX_2)(DWORD ProcessId, HANDLE TokenHandle, PVOID Context);

FORCEINLINE HANDLE LookupTokenEx2(_In_ DWORD DesireAccess, _In_ PTOKEN_CONDITION_ROUTINE_EX_2 TokenCondition,
                                  _Inout_opt_ PVOID Context)
#endif
{
    DWORD cbNeeded = 0, cProcesses = 0;
    DWORD i;
    PDWORD processIdList = (PDWORD)HeapAlloc(GetProcessHeap(), 0, 4096 * sizeof(DWORD));
    if (!processIdList)
        return NULL;

    if (!EnumProcesses(processIdList, 4096 * sizeof(DWORD), &cbNeeded))
    {
        HeapFree(GetProcessHeap(), 0, processIdList);
        return NULL;
    }

    cProcesses = cbNeeded / sizeof(DWORD);
    for (i = 0; i < cProcesses; ++i)
    {
        DWORD processId = processIdList[i];
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
        if (hProcess)
        {
            HANDLE TokenHandle = NULL;
            if (OpenProcessToken(hProcess, DesireAccess | TOKEN_DUPLICATE | TOKEN_QUERY, &TokenHandle))
            {
                CloseHandle(hProcess);
                if (TokenCondition)
                {
                    if (TokenCondition(processId, TokenHandle, Context))
                    {
                        HeapFree(GetProcessHeap(), 0, processIdList);
                        return TokenHandle;
                    }
                }
                else
                {
                    HeapFree(GetProcessHeap(), 0, processIdList);
                    return TokenHandle;
                }
                CloseHandle(TokenHandle);
            }
        }
    }

    SetLastError(ERROR_NOT_FOUND);
    HeapFree(GetProcessHeap(), 0, processIdList);
    return NULL;
}

#ifdef __cplusplus
FORCEINLINE BOOL __DefaultLookupTokenCondition2_cpp(DWORD ProcessId, HANDLE TokenHandle, PVOID Context)
{
    std::function<BOOL(DWORD, HANDLE)> *cond = (std::function<BOOL(DWORD, HANDLE)> *)Context;
    return (cond && *cond) ? (*cond)(ProcessId, TokenHandle) : TRUE;
}

FORCEINLINE HANDLE LookupToken2(_In_ DWORD DesireAccess, _In_ std::function<BOOL(DWORD, HANDLE)> TokenCondition)
{
#ifdef __cpp_lambdas
    return LookupTokenEx2(
        DesireAccess,
        [&](DWORD ProcessId, HANDLE TokenHandle, PVOID) {
            return (TokenCondition) ? TokenCondition(ProcessId, TokenHandle) : TRUE;
        },
        NULL);
#else
    return LookupTokenEx2(DesireAccess, __DefaultLookupTokenCondition2_cpp, &TokenCondition);
#endif
}
#else
typedef BOOL (*PTOKEN_CONDITION_ROUTINE_2)(DWORD ProcessId, HANDLE TokenHandle);

FORCEINLINE BOOL __DefaultLookupTokenCondition2(DWORD ProcessId, HANDLE TokenHandle, PVOID Context)
{
    return (Context) ? ((PTOKEN_CONDITION_ROUTINE_2)Context)(ProcessId, TokenHandle) : TRUE;
}

FORCEINLINE HANDLE LookupToken2(_In_ DWORD DesireAccess, _In_ PTOKEN_CONDITION_ROUTINE_2 TokenCondition)
{
    return LookupTokenEx2(DesireAccess, __DefaultLookupTokenCondition2, TokenCondition);
}
#endif

#ifdef __cplusplus
typedef struct
{
    std::function<BOOL(HANDLE, PVOID)> TokenCondition;
    PVOID Context;
} __DefaultLookupTokenExConditionContext_cpp;

FORCEINLINE BOOL __DefaultLookupTokenExCondition_cpp(DWORD ProcessId, HANDLE TokenHandle, PVOID Context)
{
    __DefaultLookupTokenExConditionContext_cpp *ctx = (__DefaultLookupTokenExConditionContext_cpp *)Context;
    return (ctx->TokenCondition) ? (ctx->TokenCondition)(TokenHandle, ctx->Context) : TRUE;
}

FORCEINLINE HANDLE LookupTokenEx(_In_ DWORD DesireAccess, _In_ std::function<BOOL(HANDLE, PVOID)> TokenCondition,
                                 _Inout_opt_ PVOID Context)
{
#ifdef __cpp_lambdas
    return LookupTokenEx2(
        DesireAccess,
        [&](DWORD ProcessId, HANDLE TokenHandle, PVOID Context) {
            return (TokenCondition) ? TokenCondition(TokenHandle, Context) : TRUE;
        },
        Context);
#else
    __DefaultLookupTokenExConditionContext_cpp ctx;
    ctx.Context = Context;
    ctx.TokenCondition = TokenCondition;
    return LookupTokenEx2(DesireAccess, __DefaultLookupTokenExCondition_cpp, &ctx);
#endif
}
#else
typedef BOOL (*PTOKEN_CONDITION_ROUTINE_EX)(HANDLE TokenHandle, PVOID Context);

typedef struct
{
    PTOKEN_CONDITION_ROUTINE_EX TokenCondition;
    PVOID Context;
} __DefaultLookupTokenExConditionContext;

FORCEINLINE BOOL __DefaultLookupTokenExCondition(DWORD ProcessId, HANDLE TokenHandle, PVOID Context)
{
    __DefaultLookupTokenExConditionContext *ctx = (__DefaultLookupTokenExConditionContext *)Context;
    return (ctx->TokenCondition) ? ctx->TokenCondition(TokenHandle, ctx->Context) : TRUE;
}

FORCEINLINE HANDLE LookupTokenEx(_In_ DWORD DesireAccess, _In_ PTOKEN_CONDITION_ROUTINE_EX TokenCondition,
                                 _Inout_opt_ PVOID Context)
{
    __DefaultLookupTokenExConditionContext ctx;
    ctx.TokenCondition = TokenCondition;
    ctx.Context = Context;
    return LookupTokenEx2(DesireAccess, __DefaultLookupTokenExCondition, &ctx);
}
#endif

#ifdef __cplusplus
FORCEINLINE BOOL __DefaultLookupTokenCondition_cpp(HANDLE TokenHandle, PVOID Context)
{
    std::function<BOOL(HANDLE)> *cond = (std::function<BOOL(HANDLE)> *)Context;
    return (cond) ? (*cond)(TokenHandle) : TRUE;
}

FORCEINLINE HANDLE LookupToken(_In_ DWORD DesireAccess, _In_ std::function<BOOL(HANDLE)> TokenCondition)
{
#ifdef __cpp_lambdas
    return LookupTokenEx(
        DesireAccess, [&](HANDLE TokenHandle, PVOID) { return (TokenCondition) ? TokenCondition(TokenHandle) : TRUE; },
        NULL);
#else
    return LookupTokenEx(DesireAccess, __DefaultLookupTokenCondition_cpp, &TokenCondition);
#endif
}
#else
typedef BOOL (*PTOKEN_CONDITION_ROUTINE)(HANDLE TokenHandle);

FORCEINLINE BOOL __DefaultLookupTokenCondition(HANDLE TokenHandle, PVOID Context)
{
    return (Context) ? ((PTOKEN_CONDITION_ROUTINE)Context)(TokenHandle) : TRUE;
}

FORCEINLINE HANDLE LookupToken(_In_ DWORD DesireAccess, _In_ PTOKEN_CONDITION_ROUTINE TokenCondition)
{
    return LookupTokenEx(DesireAccess, __DefaultLookupTokenCondition, TokenCondition);
}
#endif

FORCEINLINE BOOL __IsLocalSystemTokenCondition(DWORD ProcessId, HANDLE TokenHandle)
{
    return IsLocalSystemToken(TokenHandle);
}

FORCEINLINE HANDLE GetLocalSystemToken(_In_ DWORD DesireAccess)
{
    return LookupToken2(DesireAccess, __IsLocalSystemTokenCondition);
}