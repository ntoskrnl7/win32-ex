#pragma once

#ifdef __cplusplus
#include "..\Security/Privilege.hpp"
#else
#include "..\Security/Privilege.h"
#endif

#include "Ntdll.h"

inline BOOL IsPermanentObject(_In_ HANDLE Handle)
{
    PUBLIC_OBJECT_BASIC_INFORMATION basicInfo;
    if (NT_SUCCESS(NtQueryObject(Handle, ObjectBasicInformation, &basicInfo, sizeof(basicInfo), NULL)))
    {
        return (basicInfo.Attributes & OBJ_PERMANENT);
    }
    return FALSE;
}

inline BOOL IsTemporaryObject(_In_ HANDLE Handle)
{
    return IsPermanentObject(Handle) == FALSE;
}

inline BOOL __IsPrivilegeEnabled(_In_ DWORD ProcessId, _In_ HANDLE hToken, _In_ PVOID Context)
{
    BOOL result = FALSE;
    PPRIVILEGE_SET privilegeSet = (PPRIVILEGE_SET)Context;
    PrivilegeCheck(hToken, privilegeSet, &result);
    return result;
}

inline BOOL __FindLocalSystemTokenAndAcquireCreatePermanentPrivilege(_In_ DWORD ProcessId, _In_ HANDLE hToken,
                                                                     _In_ PVOID Context)
{
    if (!IsLocalSystemToken(hToken))
        return FALSE;
    return EnablePrivilege(TRUE, SE_CREATE_PERMANENT_NAME, (PREVIOUS_TOKEN_PRIVILEGES *)Context, hToken);
}

inline BOOL MakePermanentObject(_In_ HANDLE Handle)
{
    BOOL privilegeAqcuired = FALSE;

    //
    //  영구 공유 객체 생성 권한을 할당합니다.
    //

    PREVIOUS_TOKEN_PRIVILEGES oldState;
    PRIVILEGE_SET privilegeSet;

    HANDLE hToken;
    HANDLE hNewToken = NULL;

    NTSTATUS status;

    //
    //  영구 공유 객체 생성 권한을 획득합니다
    //

    privilegeAqcuired = EnablePrivilege(TRUE, SE_CREATE_PERMANENT_NAME, &oldState, NULL);
    if (privilegeAqcuired)
    {
        if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
        {
            //
            //  로컬 정책 -> 사용자 권한 할당 -> 영구 공유 객체 생성은 아무 계정도 할당 받지 못하도록 되어있어서 시스템
            //  토큰만 할당 가능하므로
            // 현재 프로세스가 서비스와 같은 시스템 프로세스가 아니라면 함수는 성공하지만, 실질적으로 영구 공유 객체
            // 생성 권한은 할당되지 못합니다.
            //

            privilegeAqcuired = FALSE;
            RevertPrivileges(&oldState);
        }
    }

    //
    //  현재 프로세스/스레드 토큰에 영구 전역 객체 생성 권한이 있는지 확인합니다.
    //

    privilegeSet.PrivilegeCount = 1;
    privilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;

#ifdef __cplusplus
    privilegeSet.Privilege[0].Luid = Win32Ex::Security::SeCreatePermanentPrivilege;
#else
    if (!LookupPrivilegeValue(NULL, SE_CREATE_PERMANENT_NAME, &privilegeSet.Privilege[0].Luid))
    {
        RevertPrivileges(&oldState);
        return FALSE;
    }
#endif
    privilegeSet.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!privilegeAqcuired)
    {
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, &hToken))
        {
            PrivilegeCheck(hToken, &privilegeSet, &privilegeAqcuired);
            CloseHandle(hToken);
        }

        if (!privilegeAqcuired)
        {
            if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, FALSE, &hToken))
            {
                PrivilegeCheck(hToken, &privilegeSet, &privilegeAqcuired);
                CloseHandle(hToken);
            }
        }
    }

    //
    //  현재 프로세스/스레드에 권한이 없다면, 영구 객체 생성 권한이 있는 로컬 시스템 토큰을 획득합니다.
    //

    if (!privilegeAqcuired)
    {
        PREVIOUS_TOKEN_PRIVILEGES oldDebugState;
        EnablePrivilege(TRUE, SE_DEBUG_NAME, &oldDebugState, NULL);

#if defined(__cplusplus) && defined(__cpp_lambdas)
        hToken = LookupToken2(MAXIMUM_ALLOWED, [&](DWORD, HANDLE hToken) -> BOOL {
            BOOL result = FALSE;
            PrivilegeCheck(hToken, &privilegeSet, &result);
            return result;
        });
#else
        hToken = LookupTokenEx2(MAXIMUM_ALLOWED, __IsPrivilegeEnabled, &privilegeSet);
#endif
        if (!hToken)
        {
            //
            //  영구 객체 생성 권한이 있는 토큰이 없다면, 아무 로컬 시스템 토큰을 획득 후, 권한을 할당합니다.
            //

            hToken =
                LookupTokenEx2(MAXIMUM_ALLOWED, __FindLocalSystemTokenAndAcquireCreatePermanentPrivilege, &oldState);
        }
        RevertPrivileges(&oldDebugState);

        if (hToken == NULL)
            return FALSE;

        if (!DuplicateToken(hToken, SecurityImpersonation, &hNewToken))
        {
            CloseHandle(hToken);
            return FALSE;
        }

        CloseHandle(hToken);

        if (!SetThreadToken(NULL, hNewToken))
        {
            CloseHandle(hNewToken);
            return FALSE;
        }
    }

    //
    //  영구 전역 객체로 설정합니다.
    //

    status = NtMakePermanentObject(Handle);

    if (hNewToken)
    {
        (VOID) SetThreadToken(NULL, NULL);
        CloseHandle(hNewToken);
    }

    if (privilegeAqcuired)
    {
        RevertPrivileges(&oldState);
    }

    return NT_SUCCESS(status);
}

inline BOOL MakeTemporaryObject(_In_ HANDLE Handle)
{
    return NT_SUCCESS(NtMakeTemporaryObject(Handle));
}