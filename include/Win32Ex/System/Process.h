/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Ntdll.h

Abstract:

    This Module implements the process helper procedures.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SYSTEM_PROCESS_H_
#define _WIN32EX_SYSTEM_PROCESS_H_

#include "../Internal/version.h"
#define WIN32EX_SYSTEM_PROCESS_H_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SYSTEM_PROCESS_H_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SYSTEM_PROCESS_H_VERSION_PATCH WIN32EX_VERSION_PATCH

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <tlhelp32.h>

#include <wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

#include <userenv.h>
#pragma comment(lib, "Userenv.lib")

#include "../Security/Privilege.h"
#include "../Security/Token.h"

#if defined(__cplusplus)
#include "../TmplApi/processthreadsapi.hpp"

template <typename _CharType>
STATIC_OR_INLINE BOOL CreateUserAccountProcessT(
    _In_opt_ DWORD dwSessionId, _In_opt_ const _CharType *lpApplicationName, _Inout_opt_ _CharType *lpCommandLine,
    _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes, _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
    _In_opt_ const _CharType *lpCurrentDirectory, _In_ typename Win32Ex::STARTUPINFOT<_CharType>::Type *lpStartupInfo,
    _Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
    BOOL bRet = FALSE;
    HANDLE hToken = NULL;
    HANDLE hNewToken = NULL;
    PVOID lpEnvironmentNew = NULL;
    BOOL isLocalSystemToken = FALSE;
    BOOL runningWithLocalSystem = FALSE;
    PREVIOUS_TOKEN_PRIVILEGES prevTokenPrivileges;
    DWORD currentSessionId;
    DWORD lastError = ERROR_FUNCTION_FAILED;

    //
    //  현재 프로세스의 세션과 같은 세션에 실행하려는것이라면, CreateProcess를 호출합니다.
    //
    if (ProcessIdToSessionId(GetCurrentProcessId(), &currentSessionId))
    {
        if (dwSessionId == currentSessionId)
        {
            return Win32Ex::CreateProcessT<_CharType>(
                lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
                dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
        }
    }

    //
    //  현재 프로세스가 로컬 시스템 계정으로 실행되는지 확인합니다.
    //
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        runningWithLocalSystem = isLocalSystemToken = IsLocalSystemToken(hToken);
        CloseHandle(hToken);
        hToken = NULL;
        if (!isLocalSystemToken)
        {
            if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken))
            {
                isLocalSystemToken = IsLocalSystemToken(hToken);
                CloseHandle(hToken);
                hToken = NULL;
            }
        }
    }

    //
    //  로컬 시스템 계정이 아니라면, 로컬 시스템 계정 토큰을 획득하여 스레드에 적용합니다.
    //
    if (!isLocalSystemToken)
    {
        DWORD sessionId = 0;
        hToken = GetLocalSystemToken(TOKEN_DUPLICATE);
        if (hToken == NULL)
        {
            return FALSE;
        }

        if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityAnonymous, TokenPrimary, &hNewToken))
        {
            CloseHandle(hToken);
            return FALSE;
        }

        CloseHandle(hToken);
        hToken = NULL;

        if (!ImpersonateLoggedOnUser(hNewToken))
        {
            CloseHandle(hNewToken);
            return FALSE;
        }

        if (!SetTokenInformation(hNewToken, TokenSessionId, &sessionId, sizeof(sessionId)))
        {
            CloseHandle(hNewToken);
            return FALSE;
        }

        CloseHandle(hNewToken);
    }

    //
    //  TCB 권한을 획득 후, WTSQueryUserToken를 호출하여 사용자 토큰을 획득합니다.
    //
    if (!EnablePrivilege(TRUE, SE_TCB_NAME, &prevTokenPrivileges, NULL))
    {
        if (hToken)
        {
            CloseHandle(hToken);
        }
        return FALSE;
    }

    if (!WTSQueryUserToken(dwSessionId, &hToken))
    {
        return FALSE;
    }

    RevertPrivileges(&prevTokenPrivileges);

    //
    //  사용자 토큰을 복제 후 필요한 권한을 획득하고 CreateProcessAsUser를 호출합니다.
    //

    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hNewToken))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);

    if (!EnablePrivilegesV(TRUE, &prevTokenPrivileges, hNewToken, 2, SE_ASSIGNPRIMARYTOKEN_NAME,
                           SE_INCREASE_QUOTA_NAME))
    {
        RevertPrivileges(&prevTokenPrivileges);
        CloseHandle(hNewToken);
        return FALSE;
    }

    //
    //  서비스 프로세스(로컬 시스템 계정)의 경우,
    // 토큰에 권한을 획득하지 못해도 CreateProcessAsUser가 정상 수행되므로
    // 권한을 획득하지 못해도 작업을 진행합니다.
    //

    if (!runningWithLocalSystem && GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        CloseHandle(hNewToken);
        return FALSE;
    }

    if (!lpEnvironment)
    {
        if (CreateEnvironmentBlock(&lpEnvironmentNew, hNewToken, FALSE))
        {
            SetFlag(dwCreationFlags, CREATE_UNICODE_ENVIRONMENT);
        }
    }

    bRet = Win32Ex::CreateProcessAsUserT<_CharType>(hNewToken, lpApplicationName, lpCommandLine, lpProcessAttributes,
                                                    lpThreadAttributes, bInheritHandles, dwCreationFlags,
                                                    lpEnvironmentNew ? lpEnvironmentNew : lpEnvironment,
                                                    lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

    if (!bRet)
    {
        lastError = GetLastError();
    }

    //
    //  할당한 자원을 해제합니다.
    //

    if (lpEnvironmentNew)
    {
        DestroyEnvironmentBlock(lpEnvironmentNew);
    }

    if (!isLocalSystemToken)
    {
        RevertToSelf();
    }

    RevertPrivileges(&prevTokenPrivileges);

    CloseHandle(hNewToken);

    if (!bRet)
    {
        SetLastError(lastError);
    }

    return bRet;
}

template <typename _CharType>
STATIC_OR_INLINE BOOL CreateSystemAccountProcessT(
    _In_opt_ DWORD dwSessionId, _In_opt_ const _CharType *lpApplicationName, _Inout_opt_ _CharType *lpCommandLine,
    _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes, _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
    _In_opt_ const _CharType *lpCurrentDirectory, _In_ typename Win32Ex::STARTUPINFOT<_CharType>::Type *lpStartupInfo,
    _Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
    HANDLE hToken;
    HANDLE hNewToken = NULL;
    PVOID lpEnvironmentNew = NULL;

    DWORD returnedLen = 0;
    DWORD origSessionId = 0;

    BOOL bRet;
    PREVIOUS_TOKEN_PRIVILEGES prevTokenPrivileges;
    DWORD lastError;

    BOOL isLocalSystemToken = FALSE;

    //
    //  현재 프로세스가 로컬 시스템 계정으로 실행되었다면 해당 토큰을 획득합니다.
    //

    if (OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &hToken))
    {
        isLocalSystemToken = IsLocalSystemToken(hToken);

        if (!isLocalSystemToken)
        {
            CloseHandle(hToken);

            if (OpenThreadToken(GetCurrentThread(), MAXIMUM_ALLOWED, FALSE, &hToken))
            {
                isLocalSystemToken = IsLocalSystemToken(hToken);
                if (!isLocalSystemToken)
                {
                    CloseHandle(hToken);
                }
            }
        }
    }

    //
    //  로컬 시스템 계정이 아니라면, 로컬 시스템 계정 토큰을 획득합니다.
    //

    if (!isLocalSystemToken)
    {
        DWORD sessionId = 0;

        //
        //  로컬 시스템 토큰을 획득합니다.
        //

        if (!EnablePrivilege(TRUE, SE_DEBUG_NAME, &prevTokenPrivileges, NULL))
        {
            return FALSE;
        }

        hToken = GetLocalSystemToken(TOKEN_DUPLICATE);

        RevertPrivileges(&prevTokenPrivileges);

        if (hToken == NULL)
        {
            return FALSE;
        }

        //
        //  로컬 시스템 계정으로 실행한것과 같은 상태로 토큰을 설정합니다.
        // (실제 로컬 시스템 계정으로 실행했을때의 토큰 상태와 흡사하도록 생성했습니다, Windows 10 1809 기준)
        //

        if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityAnonymous, TokenPrimary, &hNewToken))
        {
            CloseHandle(hToken);
            return FALSE;
        }

        CloseHandle(hToken);

        if (!ImpersonateLoggedOnUser(hNewToken))
        {
            CloseHandle(hNewToken);
            return FALSE;
        }

        if (!SetTokenInformation(hNewToken, TokenSessionId, &sessionId, sizeof(sessionId)))
        {
            CloseHandle(hNewToken);
            return FALSE;
        }

        hToken = hNewToken;
    }

    //
    //  시스템 토큰 핸들을 복제 후 대상 세션 식별자로 변경 후 CreateProcessAsUser를 호출합니다.
    //

    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hNewToken))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);

    if (!EnablePrivilegesV(TRUE, &prevTokenPrivileges, hNewToken, 2, SE_INCREASE_QUOTA_NAME,
                           SE_ASSIGNPRIMARYTOKEN_NAME))
    {
        CloseHandle(hNewToken);
        return FALSE;
    }

    if (!GetTokenInformation(hNewToken, TokenSessionId, &origSessionId, sizeof(origSessionId), &returnedLen))
    {
        RevertPrivileges(&prevTokenPrivileges);
        CloseHandle(hNewToken);
        return FALSE;
    }

    if (!ImpersonateLoggedOnUser(hNewToken))
    {
        RevertPrivileges(&prevTokenPrivileges);
        CloseHandle(hNewToken);
        return FALSE;
    }

    if (!SetTokenInformation(hNewToken, TokenSessionId, &dwSessionId, sizeof(dwSessionId)))
    {
        RevertPrivileges(&prevTokenPrivileges);
        CloseHandle(hNewToken);
        return FALSE;
    }

    if (lpEnvironment)
    {
        if (CreateEnvironmentBlock(&lpEnvironmentNew, hNewToken, FALSE))
        {
            SetFlag(dwCreationFlags, CREATE_UNICODE_ENVIRONMENT);
        }
    }

    bRet = Win32Ex::CreateProcessAsUserT<_CharType>(hNewToken, lpApplicationName, lpCommandLine, lpProcessAttributes,
                                                    lpThreadAttributes, bInheritHandles, dwCreationFlags,
                                                    lpEnvironmentNew ? lpEnvironmentNew : lpEnvironment,
                                                    lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

    if (!bRet)
    {
        lastError = GetLastError();
    }

    //
    //  할당한 자원을 해제합니다.
    //

    if (lpEnvironmentNew)
    {
        DestroyEnvironmentBlock(lpEnvironmentNew);
    }

    if (!SetTokenInformation(hNewToken, TokenSessionId, &origSessionId, sizeof(origSessionId)))
    {
        CloseHandle(hNewToken);
        return FALSE;
    }

    RevertToSelf();

    RevertPrivileges(&prevTokenPrivileges);

    CloseHandle(hNewToken);

    if (!bRet)
    {
        SetLastError(lastError);
    }

    return bRet;
}

#ifdef UNICODE
#define CreateUserAccountProcess CreateUserAccountProcessT<WCHAR>
#define CreateSystemAccountProcess CreateSystemAccountProcessT<WCHAR>
#else
#define CreateUserAccountProcess CreateUserAccountProcessT<CHAR>
#define CreateSystemAccountProcess CreateSystemAccountProcessT<CHAR>
#endif // !UNICODE
#else
STATIC_OR_INLINE BOOL CreateUserAccountProcess(_In_opt_ DWORD dwSessionId, _In_opt_ LPCTSTR lpApplicationName,
                                               _Inout_opt_ LPTSTR lpCommandLine,
                                               _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                               _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                               _In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags,
                                               _In_opt_ LPVOID lpEnvironment, _In_opt_ LPCTSTR lpCurrentDirectory,
                                               _In_ LPSTARTUPINFO lpStartupInfo,
                                               _Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
    BOOL bRet = FALSE;
    HANDLE hToken = NULL;
    HANDLE hNewToken = NULL;
    PVOID lpEnvironmentNew = NULL;
    BOOL isLocalSystemToken = FALSE;
    BOOL runningWithLocalSystem = FALSE;
    PREVIOUS_TOKEN_PRIVILEGES prevTokenPrivileges;
    DWORD currentSessionId;
    DWORD lastError = ERROR_FUNCTION_FAILED;

    //
    //  현재 프로세스의 세션과 같은 세션에 실행하려는것이라면, CreateProcess를 호출합니다.
    //
    if (ProcessIdToSessionId(GetCurrentProcessId(), &currentSessionId))
    {
        if (dwSessionId == currentSessionId)
        {
            return CreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                                 bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
                                 lpProcessInformation);
        }
    }

    //
    //  현재 프로세스가 로컬 시스템 계정으로 실행되는지 확인합니다.
    //
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        runningWithLocalSystem = isLocalSystemToken = IsLocalSystemToken(hToken);
        CloseHandle(hToken);
        hToken = NULL;
        if (!isLocalSystemToken)
        {
            if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken))
            {
                isLocalSystemToken = IsLocalSystemToken(hToken);
                CloseHandle(hToken);
                hToken = NULL;
            }
        }
    }

    //
    //  로컬 시스템 계정이 아니라면, 로컬 시스템 계정 토큰을 획득하여 스레드에 적용합니다.
    //
    if (!isLocalSystemToken)
    {
        DWORD sessionId = 0;
        hToken = GetLocalSystemToken(TOKEN_DUPLICATE);
        if (hToken == NULL)
        {
            return FALSE;
        }

        if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityAnonymous, TokenPrimary, &hNewToken))
        {
            CloseHandle(hToken);
            return FALSE;
        }

        CloseHandle(hToken);
        hToken = NULL;

        if (!ImpersonateLoggedOnUser(hNewToken))
        {
            CloseHandle(hNewToken);
            return FALSE;
        }

        if (!SetTokenInformation(hNewToken, TokenSessionId, &sessionId, sizeof(sessionId)))
        {
            CloseHandle(hNewToken);
            return FALSE;
        }

        CloseHandle(hNewToken);
    }

    //
    //  TCB 권한을 획득 후, WTSQueryUserToken를 호출하여 사용자 토큰을 획득합니다.
    //
    if (!EnablePrivilege(TRUE, SE_TCB_NAME, &prevTokenPrivileges, NULL))
    {
        if (hToken)
        {
            CloseHandle(hToken);
        }
        return FALSE;
    }

    if (!WTSQueryUserToken(dwSessionId, &hToken))
    {
        return FALSE;
    }

    RevertPrivileges(&prevTokenPrivileges);

    //
    //  사용자 토큰을 복제 후 필요한 권한을 획득하고 CreateProcessAsUser를 호출합니다.
    //

    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hNewToken))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);

    if (!EnablePrivilegesV(TRUE, &prevTokenPrivileges, hNewToken, 2, SE_ASSIGNPRIMARYTOKEN_NAME,
                           SE_INCREASE_QUOTA_NAME))
    {
        RevertPrivileges(&prevTokenPrivileges);
        CloseHandle(hNewToken);
        return FALSE;
    }

    //
    //  서비스 프로세스(로컬 시스템 계정)의 경우,
    // 토큰에 권한을 획득하지 못해도 CreateProcessAsUser가 정상 수행되므로
    // 권한을 획득하지 못해도 작업을 진행합니다.
    //

    if (!runningWithLocalSystem && GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        CloseHandle(hNewToken);
        return FALSE;
    }

    if (!lpEnvironment)
    {
        if (CreateEnvironmentBlock(&lpEnvironmentNew, hNewToken, FALSE))
        {
            SetFlag(dwCreationFlags, CREATE_UNICODE_ENVIRONMENT);
        }
    }

    bRet = CreateProcessAsUser(hNewToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                               bInheritHandles, dwCreationFlags, lpEnvironmentNew ? lpEnvironmentNew : lpEnvironment,
                               lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

    if (!bRet)
    {
        lastError = GetLastError();
    }

    //
    //  할당한 자원을 해제합니다.
    //

    if (lpEnvironmentNew)
    {
        DestroyEnvironmentBlock(lpEnvironmentNew);
    }

    if (!isLocalSystemToken)
    {
        RevertToSelf();
    }

    RevertPrivileges(&prevTokenPrivileges);

    CloseHandle(hNewToken);

    if (!bRet)
    {
        SetLastError(lastError);
    }

    return bRet;
}

STATIC_OR_INLINE BOOL CreateSystemAccountProcess(_In_opt_ DWORD dwSessionId, _In_opt_ LPCTSTR lpApplicationName,
                                                 _Inout_opt_ LPTSTR lpCommandLine,
                                                 _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                                 _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                                 _In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags,
                                                 _In_opt_ LPVOID lpEnvironment, _In_opt_ LPCTSTR lpCurrentDirectory,
                                                 _In_ LPSTARTUPINFO lpStartupInfo,
                                                 _Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
    HANDLE hToken;
    HANDLE hNewToken = NULL;
    PVOID lpEnvironmentNew = NULL;

    DWORD returnedLen = 0;
    DWORD origSessionId = 0;

    BOOL bRet;
    PREVIOUS_TOKEN_PRIVILEGES prevTokenPrivileges;
    DWORD lastError;

    BOOL isLocalSystemToken = FALSE;

    //
    //  현재 프로세스가 로컬 시스템 계정으로 실행되었다면 해당 토큰을 획득합니다.
    //

    if (OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &hToken))
    {
        isLocalSystemToken = IsLocalSystemToken(hToken);

        if (!isLocalSystemToken)
        {
            CloseHandle(hToken);

            if (OpenThreadToken(GetCurrentThread(), MAXIMUM_ALLOWED, FALSE, &hToken))
            {
                isLocalSystemToken = IsLocalSystemToken(hToken);
                if (!isLocalSystemToken)
                {
                    CloseHandle(hToken);
                }
            }
        }
    }

    //
    //  로컬 시스템 계정이 아니라면, 로컬 시스템 계정 토큰을 획득합니다.
    //

    if (!isLocalSystemToken)
    {
        DWORD sessionId = 0;

        //
        //  로컬 시스템 토큰을 획득합니다.
        //

        if (!EnablePrivilege(TRUE, SE_DEBUG_NAME, &prevTokenPrivileges, NULL))
        {
            return FALSE;
        }

        hToken = GetLocalSystemToken(TOKEN_DUPLICATE);

        RevertPrivileges(&prevTokenPrivileges);

        if (hToken == NULL)
        {
            return FALSE;
        }

        //
        //  로컬 시스템 계정으로 실행한것과 같은 상태로 토큰을 설정합니다.
        // (실제 로컬 시스템 계정으로 실행했을때의 토큰 상태와 흡사하도록 생성했습니다, Windows 10 1809 기준)
        //

        if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityAnonymous, TokenPrimary, &hNewToken))
        {
            CloseHandle(hToken);
            return FALSE;
        }

        CloseHandle(hToken);

        if (!ImpersonateLoggedOnUser(hNewToken))
        {
            CloseHandle(hNewToken);
            return FALSE;
        }

        if (!SetTokenInformation(hNewToken, TokenSessionId, &sessionId, sizeof(sessionId)))
        {
            CloseHandle(hNewToken);
            return FALSE;
        }

        hToken = hNewToken;
    }

    //
    //  시스템 토큰 핸들을 복제 후 대상 세션 식별자로 변경 후 CreateProcessAsUser를 호출합니다.
    //

    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hNewToken))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);

    if (!EnablePrivilegesV(TRUE, &prevTokenPrivileges, hNewToken, 2, SE_INCREASE_QUOTA_NAME,
                           SE_ASSIGNPRIMARYTOKEN_NAME))
    {
        CloseHandle(hNewToken);
        return FALSE;
    }

    if (!GetTokenInformation(hNewToken, TokenSessionId, &origSessionId, sizeof(origSessionId), &returnedLen))
    {
        RevertPrivileges(&prevTokenPrivileges);
        CloseHandle(hNewToken);
        return FALSE;
    }

    if (!ImpersonateLoggedOnUser(hNewToken))
    {
        RevertPrivileges(&prevTokenPrivileges);
        CloseHandle(hNewToken);
        return FALSE;
    }

    if (!SetTokenInformation(hNewToken, TokenSessionId, &dwSessionId, sizeof(dwSessionId)))
    {
        RevertPrivileges(&prevTokenPrivileges);
        CloseHandle(hNewToken);
        return FALSE;
    }

    if (lpEnvironment)
    {
        if (CreateEnvironmentBlock(&lpEnvironmentNew, hNewToken, FALSE))
        {
            SetFlag(dwCreationFlags, CREATE_UNICODE_ENVIRONMENT);
        }
    }

    bRet = CreateProcessAsUser(hNewToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                               bInheritHandles, dwCreationFlags, lpEnvironmentNew ? lpEnvironmentNew : lpEnvironment,
                               lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

    if (!bRet)
    {
        lastError = GetLastError();
    }

    //
    //  할당한 자원을 해제합니다.
    //

    if (lpEnvironmentNew)
    {
        DestroyEnvironmentBlock(lpEnvironmentNew);
    }

    if (!SetTokenInformation(hNewToken, TokenSessionId, &origSessionId, sizeof(origSessionId)))
    {
        CloseHandle(hNewToken);
        return FALSE;
    }

    RevertToSelf();

    RevertPrivileges(&prevTokenPrivileges);

    CloseHandle(hNewToken);

    if (!bRet)
    {
        SetLastError(lastError);
    }

    return bRet;
}

#endif

STATIC_OR_INLINE DWORD GetParentProcessId(DWORD ProcessId)
{
    DWORD parentProcessId = 0;

    HANDLE hSnapshot;
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return parentProcessId;

    for (;;)
    {
        PROCESSENTRY32 pe32 = {
            0,
        };
        pe32.dwSize = sizeof(pe32);

        if (!Process32First(hSnapshot, &pe32))
            break;
        do
        {
            if (pe32.th32ProcessID == ProcessId)
            {
                parentProcessId = pe32.th32ParentProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
        break;
    }

    CloseHandle(hSnapshot);
    return parentProcessId;
}

#endif // _WIN32EX_SYSTEM_PROCESS_H_