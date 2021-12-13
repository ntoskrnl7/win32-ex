/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Privilege.h

Abstract:

    This Module implements the privilege procedures.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SECURITY_PRIVILEGE_H_
#define _WIN32EX_SECURITY_PRIVILEGE_H_

#include "../Internal/version.h"
#define WIN32EX_SECURITY_PRIVILEGE_H_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SECURITY_PRIVILEGE_H_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SECURITY_PRIVILEGE_H_VERSION_PATCH WIN32EX_VERSION_PATCH

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <Windows.h>

#include <stdlib.h>

#include "../Internal/misc.h"
#include "Token.h"

FORCEINLINE PTOKEN_PRIVILEGES GetTokenPrivileges(_In_ HANDLE TokenHandle)
{
    return (PTOKEN_PRIVILEGES)GetTokenInfo(TokenHandle, TokenPrivileges, NULL);
}

FORCEINLINE VOID FreeTokenPrivileges(_In_ PTOKEN_PRIVILEGES TokenPrivileges)
{
    HeapFree(GetProcessHeap(), 0, TokenPrivileges);
}

FORCEINLINE BOOL LookupPrivilegesValue(_In_ DWORD NumberOfPrivilegeNames, _In_ CONST LPCTSTR PrivilegeNames[],
                                       _Inout_ LUID Luids[])
{
    DWORD i;
    for (i = 0; i < NumberOfPrivilegeNames; ++i)
    {
        if (!LookupPrivilegeValue(NULL, PrivilegeNames[i], &Luids[i]))
            return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
//                                                                    //
//               NT Defined Privileges                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////
#ifdef UNICODE
#define SE_ASSIGNPRIMARYTOKEN_NAME_W SE_ASSIGNPRIMARYTOKEN_NAME
#define SE_INCREASE_QUOTA_NAME_W SE_INCREASE_QUOTA_NAME
#define SE_TCB_NAME_W SE_SE_TCB_NAME
#define SE_LOAD_DRIVER_NAME_W SE_LOAD_DRIVER_NAME
#define SE_BACKUP_NAME_W SE_BACKUP_NAME
#define SE_RESTORE_NAME_W SE_RESTORE_NAME
#define SE_SHUTDOWN_NAME_W SE_SHUTDOWN_NAME
#define SE_DEBUG_NAME_W SE_DEBUG_NAME
#define SE_AUDIT_NAME_W SE_AUDIT_NAME
#define SE_SYSTEM_ENVIRONMENT_NAME_W SE_SYSTEM_ENVIRONMENT_NAME
#define SE_CHANGE_NOTIFY_NAME_W SE_CHANGE_NOTIFY_NAME
#define SE_REMOTE_SHUTDOWN_NAME_W SE_REMOTE_SHUTDOWN_NAME
#define SE_IMPERSONATE_NAME_W SE_IMPERSONATE_NAME
#define SE_CREATE_GLOBAL_NAME_W SE_CREATE_GLOBAL_NAME
#else
#define SE_ASSIGNPRIMARYTOKEN_NAME_W _W(SE_ASSIGNPRIMARYTOKEN_NAME)
#define SE_INCREASE_QUOTA_NAME_W _W(SE_INCREASE_QUOTA_NAME)
#define SE_TCB_NAME_W _W(SE_SE_TCB_NAME
#define SE_LOAD_DRIVER_NAME_W _W(SE_LOAD_DRIVER_NAME)
#define SE_BACKUP_NAME_W _W(SE_BACKUP_NAME)
#define SE_RESTORE_NAME_W _W(SE_RESTORE_NAME)
#define SE_SHUTDOWN_NAME_W _W(SE_SHUTDOWN_NAME)
#define SE_DEBUG_NAME_W _W(SE_DEBUG_NAME)
#define SE_AUDIT_NAME_W _W(SE_AUDIT_NAME)
#define SE_SYSTEM_ENVIRONMENT_NAME_W _W(SE_SYSTEM_ENVIRONMENT_NAME)
#define SE_CHANGE_NOTIFY_NAME_W _W(SE_CHANGE_NOTIFY_NAME)
#define SE_REMOTE_SHUTDOWN_NAME_W _W(SE_REMOTE_SHUTDOWN_NAME)
#define SE_IMPERSONATE_NAME_W _W(SE_IMPERSONATE_NAME)
#define SE_CREATE_GLOBAL_NAME_W _W(SE_CREATE_GLOBAL_NAME)
#endif

#ifndef SE_MIN_WELL_KNOWN_PRIVILEGE
#define SE_MIN_WELL_KNOWN_PRIVILEGE (2L)
#define SE_MIN_WELL_KNOWN_PRIVILEGE_DEFINED
#endif
#ifndef SE_MAX_WELL_KNOWN_PRIVILEGE
#define SE_MAX_WELL_KNOWN_PRIVILEGE (36L)
#define SE_MAX_WELL_KNOWN_PRIVILEGE_DEFINED
#endif

typedef struct _PREVIOUS_TOKEN_PRIVILEGES
{
    HANDLE TokenHandle;
    PTOKEN_PRIVILEGES PreviousState;
    DWORD PreviousStateLength;
} PREVIOUS_TOKEN_PRIVILEGES, *PPREVIOUS_TOKEN_PRIVILEGES;

FORCEINLINE BOOL EnableAvailablePrivileges(_In_ BOOL Enabled, _Out_opt_ PPREVIOUS_TOKEN_PRIVILEGES PreviousPrivileges,
                                           _In_opt_ HANDLE TokenHandle)
{
    PTOKEN_PRIVILEGES tokenPrivileges;
    DWORD tokenPrivilegesLength =
        sizeof(TOKEN_PRIVILEGES) +
        (sizeof(LUID_AND_ATTRIBUTES) * (SE_MAX_WELL_KNOWN_PRIVILEGE - SE_MIN_WELL_KNOWN_PRIVILEGE));
    BOOL bCloseToken = FALSE;
    DWORD i;
    PTOKEN_PRIVILEGES previousState = NULL;
    DWORD previousStateLength;
    BOOL bRet;

    tokenPrivileges = (PTOKEN_PRIVILEGES)HeapAlloc(GetProcessHeap(), 0, tokenPrivilegesLength);
    if (!tokenPrivileges)
        return FALSE;

    if (TokenHandle == NULL)
    {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))
        {
            HeapFree(GetProcessHeap(), 0, tokenPrivileges);
            return FALSE;
        }
        bCloseToken = TRUE;
    }

    tokenPrivileges->PrivilegeCount = SE_MAX_WELL_KNOWN_PRIVILEGE - SE_MIN_WELL_KNOWN_PRIVILEGE + 1;

    for (i = 0; i < tokenPrivileges->PrivilegeCount; i++)
    {
        tokenPrivileges->Privileges[i].Luid.HighPart = 0;
        tokenPrivileges->Privileges[i].Luid.LowPart = i + SE_MIN_WELL_KNOWN_PRIVILEGE;
        tokenPrivileges->Privileges[i].Attributes = Enabled ? SE_PRIVILEGE_ENABLED : 0;
    }

    if (PreviousPrivileges)
    {
        PreviousPrivileges->PreviousState = previousState = tokenPrivileges;
        PreviousPrivileges->PreviousStateLength = tokenPrivilegesLength;
        if (bCloseToken)
        {
            PreviousPrivileges->TokenHandle = TokenHandle;
            bCloseToken = FALSE;
        }
        else
        {
            if (!DuplicateHandle(GetCurrentProcess(), TokenHandle, GetCurrentProcess(),
                                 &PreviousPrivileges->TokenHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
            {
                PreviousPrivileges = NULL;
            }
        }
    }

    bRet = AdjustTokenPrivileges(TokenHandle, FALSE, tokenPrivileges, tokenPrivilegesLength, previousState,
                                 &previousStateLength);

    if (!bRet)
    {
        if (PreviousPrivileges)
        {
            CloseHandle(PreviousPrivileges->TokenHandle);
            PreviousPrivileges = NULL;
        }
    }

    if (!PreviousPrivileges)
        HeapFree(GetProcessHeap(), 0, tokenPrivileges);

    if (bCloseToken)
        CloseHandle(TokenHandle);

    return bRet;
}

#ifdef SE_MIN_WELL_KNOWN_PRIVILEGE_DEFINED
#undef SE_MIN_WELL_KNOWN_PRIVILEGE
#undef SE_MIN_WELL_KNOWN_PRIVILEGE_DEFINED
#endif
#ifndef SE_MAX_WELL_KNOWN_PRIVILEGE_DEFINED
#undef SE_MAX_WELL_KNOWN_PRIVILEGE
#undef SE_MAX_WELL_KNOWN_PRIVILEGE_DEFINED
#endif

FORCEINLINE BOOL EnablePrivilegesEx(_In_ BOOL Enabled, _In_ DWORD NumberOfPrivileges, _In_ CONST LUID Privileges[],
                                    _Out_opt_ PPREVIOUS_TOKEN_PRIVILEGES PreviousPrivileges,
                                    _In_opt_ HANDLE TokenHandle)
{
    PTOKEN_PRIVILEGES tokenPrivileges;
    DWORD tokenPrivilegesLength = sizeof(TOKEN_PRIVILEGES) + (sizeof(LUID_AND_ATTRIBUTES) * (NumberOfPrivileges - 1));
    BOOL bCloseToken = FALSE;

    PTOKEN_PRIVILEGES previousState = NULL;
    DWORD previousStateLength;
    DWORD i;
    BOOL bRet;

    tokenPrivileges = (PTOKEN_PRIVILEGES)HeapAlloc(GetProcessHeap(), 0, tokenPrivilegesLength);
    if (!tokenPrivileges)
        return FALSE;

    if (TokenHandle == NULL)
    {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))
        {
            HeapFree(GetProcessHeap(), 0, tokenPrivileges);
            return FALSE;
        }
        bCloseToken = TRUE;
    }

    tokenPrivileges->PrivilegeCount = NumberOfPrivileges;

    for (i = 0; i < NumberOfPrivileges; i++)
    {
        tokenPrivileges->Privileges[i].Luid = Privileges[i];
        tokenPrivileges->Privileges[i].Attributes = Enabled ? SE_PRIVILEGE_ENABLED : 0;
    }

    if (PreviousPrivileges)
    {
        PreviousPrivileges->PreviousState = previousState = tokenPrivileges;
        PreviousPrivileges->PreviousStateLength = tokenPrivilegesLength;
        if (bCloseToken)
        {
            PreviousPrivileges->TokenHandle = TokenHandle;
            bCloseToken = FALSE;
        }
        else
        {
            if (!DuplicateHandle(GetCurrentProcess(), TokenHandle, GetCurrentProcess(),
                                 &PreviousPrivileges->TokenHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
            {
                PreviousPrivileges = NULL;
            }
        }
    }

    bRet = AdjustTokenPrivileges(TokenHandle, FALSE, tokenPrivileges, tokenPrivilegesLength, previousState,
                                 &previousStateLength);

    if (!(bRet && previousState && ((previousState->PrivilegeCount > 0))))
    {
        if (PreviousPrivileges)
        {
            if (PreviousPrivileges->TokenHandle)
            {
                CloseHandle(PreviousPrivileges->TokenHandle);
                PreviousPrivileges->TokenHandle = NULL;
            }
            PreviousPrivileges->PreviousState = NULL;
            PreviousPrivileges->PreviousStateLength = 0;
            PreviousPrivileges = NULL;
        }
    }

    if (!PreviousPrivileges)
        HeapFree(GetProcessHeap(), 0, tokenPrivileges);

    if (bCloseToken)
        CloseHandle(TokenHandle);

    return bRet;
}

VA_FN_INLINE BOOL EnablePrivilegesExV(_In_ BOOL Enabled, _Out_opt_ PPREVIOUS_TOKEN_PRIVILEGES PreviousPrivileges,
                                      _In_opt_ HANDLE TokenHandle, _In_ DWORD NumberOfPrivileges,
                                      /* LUID Privileges... */...)
{
    va_list Privileges;
    va_start(Privileges, NumberOfPrivileges);
    return EnablePrivilegesEx(Enabled, NumberOfPrivileges, (LUID *)Privileges, PreviousPrivileges, TokenHandle);
}

VA_FN_INLINE BOOL EnablePrivileges(_In_ BOOL Enabled, _In_ DWORD NumberOfPrivilegeNames,
                                   _In_ CONST LPCTSTR PrivilegeNames[],
                                   _Out_opt_ PPREVIOUS_TOKEN_PRIVILEGES PreviousPrivileges, _In_opt_ HANDLE TokenHandle)
{
    BOOL ret = FALSE;
    PLUID luids = (PLUID)HeapAlloc(GetProcessHeap(), 0, sizeof(LUID) * NumberOfPrivilegeNames);
    if (luids == NULL)
        return FALSE;

    if (LookupPrivilegesValue(NumberOfPrivilegeNames, PrivilegeNames, luids))
        ret = EnablePrivilegesEx(Enabled, NumberOfPrivilegeNames, luids, PreviousPrivileges, TokenHandle);

    HeapFree(GetProcessHeap(), 0, luids);
    return ret;
}

VA_FN_INLINE
BOOL EnablePrivilegesV(_In_ BOOL Enabled, _Out_opt_ PPREVIOUS_TOKEN_PRIVILEGES PreviousPrivileges,
                       _In_opt_ HANDLE TokenHandle, _In_ DWORD NumberOfPrivilegeNames,
                       /* LPCTSTR PrivilegeNames... */...)
{
    va_list PrivilegeNames;
    va_start(PrivilegeNames, NumberOfPrivilegeNames);
    return EnablePrivileges(Enabled, NumberOfPrivilegeNames, (PCTSTR *)PrivilegeNames, PreviousPrivileges, TokenHandle);
}

STATIC_OR_INLINE BOOL EnablePrivilege(_In_ BOOL Enabled, _In_ LPCTSTR PrivilegeName,
                                      _Out_opt_ PPREVIOUS_TOKEN_PRIVILEGES PreviousPrivileges,
                                      _In_opt_ HANDLE TokenHandle)
{
    return EnablePrivileges(Enabled, 1, &PrivilegeName, PreviousPrivileges, TokenHandle);
}

STATIC_OR_INLINE BOOL EnablePrivilegeEx(_In_ BOOL Enabled, _In_ LUID Privilege,
                                        _Out_opt_ PPREVIOUS_TOKEN_PRIVILEGES PreviousPrivileges,
                                        _In_opt_ HANDLE TokenHandle)
{
    return EnablePrivilegesEx(Enabled, 1, &Privilege, PreviousPrivileges, TokenHandle);
}

FORCEINLINE BOOL RevertPrivileges(_In_ PPREVIOUS_TOKEN_PRIVILEGES PreviousPrivilege)
{
    HANDLE TokenHandle = PreviousPrivilege->TokenHandle;
    PTOKEN_PRIVILEGES previousState = PreviousPrivilege->PreviousState;
    DWORD previousStateLength = PreviousPrivilege->PreviousStateLength;
    BOOL bRet = TRUE;

    if (previousState)
    {
        bRet = AdjustTokenPrivileges(TokenHandle, FALSE, previousState, previousStateLength, NULL, NULL);
        HeapFree(GetProcessHeap(), 0, previousState);
        PreviousPrivilege->PreviousState = NULL;
        PreviousPrivilege->PreviousStateLength = 0;
    }

    if (TokenHandle)
    {
        CloseHandle(TokenHandle);
        PreviousPrivilege->TokenHandle = NULL;
    }

    return bRet;
}

FORCEINLINE BOOL IsPrivilegeEnabledEx(_In_ LUID Privilege, _In_opt_ HANDLE TokenHandle)
{
    PTOKEN_PRIVILEGES privs = NULL;
    if (!TokenHandle)
        TokenHandle = GetCurrentProcessToken();

    privs = GetTokenPrivileges(TokenHandle);
    if (privs)
    {
        DWORD j;
        for (j = 0; j < privs->PrivilegeCount; ++j)
        {
            if ((Privilege.HighPart == privs->Privileges[j].Luid.HighPart &&
                 Privilege.LowPart == privs->Privileges[j].Luid.LowPart) &&
                (privs->Privileges[j].Attributes & SE_PRIVILEGE_ENABLED))
            {
                FreeTokenPrivileges(privs);
                return TRUE;
            }
        }

        FreeTokenPrivileges(privs);
    }
    return FALSE;
}

FORCEINLINE BOOL IsPrivilegeEnabled(_In_ LPCTSTR PrivilegeName, _In_opt_ HANDLE TokenHandle)
{
    LUID Privilege;
    if (!LookupPrivilegeValue(NULL, PrivilegeName, &Privilege))
        return FALSE;
    return IsPrivilegeEnabledEx(Privilege, TokenHandle);
}

FORCEINLINE BOOL IsPrivilegesEnabledEx(_In_ DWORD NumberOfPrivileges, _In_ LUID Privileges[],
                                       _In_opt_ HANDLE TokenHandle)
{
    size_t matched = 0;
    PTOKEN_PRIVILEGES privs;

    if (!TokenHandle)
        TokenHandle = GetCurrentProcessToken();

    privs = GetTokenPrivileges(TokenHandle);
    if (privs)
    {
        DWORD i;
        for (i = 0; i < NumberOfPrivileges; ++i)
        {
            PLUID Privilege = &Privileges[i];
            DWORD j;
            for (j = 0; j < privs->PrivilegeCount; ++j)
            {
                if ((Privilege->HighPart == privs->Privileges[j].Luid.HighPart &&
                     Privilege->LowPart == privs->Privileges[j].Luid.LowPart) &&
                    (privs->Privileges[j].Attributes & SE_PRIVILEGE_ENABLED))
                {
                    matched++;
                    break;
                }
            }
        }
        FreeTokenPrivileges(privs);
        return matched == NumberOfPrivileges;
    }
    return FALSE;
}

FORCEINLINE BOOL IsPrivilegesEnabled(_In_ DWORD NumberOfPrivilegeNames, _In_ LPCTSTR PrivilegeNames[],
                                     _In_opt_ HANDLE TokenHandle)
{
    BOOL ret = FALSE;
    PLUID luids = (PLUID)HeapAlloc(GetProcessHeap(), 0, sizeof(LUID) * NumberOfPrivilegeNames);
    if (luids == NULL)
        return FALSE;

    if (LookupPrivilegesValue(NumberOfPrivilegeNames, PrivilegeNames, luids))
        ret = IsPrivilegesEnabledEx(NumberOfPrivilegeNames, luids, TokenHandle);

    HeapFree(GetProcessHeap(), 0, luids);
    return ret;
}

VA_FN_INLINE BOOL IsPrivilegesEnabledV(_In_opt_ HANDLE TokenHandle, _In_ DWORD NumberOfPrivilegeNames,
                                       /* LPCTSTR PrivilegeName... */...)
{
    va_list PrivilegeName;
    va_start(PrivilegeName, NumberOfPrivilegeNames);
    return IsPrivilegesEnabled(NumberOfPrivilegeNames, (PCTSTR *)PrivilegeName, TokenHandle);
}

VA_FN_INLINE

BOOL IsPrivilegesEnabledExV(_In_opt_ HANDLE TokenHandle, _In_ DWORD NumberOfPrivileges,
                            /* LUID Privileges... */...)
{
    va_list Privileges;
    va_start(Privileges, NumberOfPrivileges);
    return IsPrivilegesEnabledEx(NumberOfPrivileges, (LUID *)Privileges, TokenHandle);
}

#endif // _WIN32EX_SECURITY_PRIVILEGE_H_