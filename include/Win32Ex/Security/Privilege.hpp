/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Privilege.hpp

Abstract:

    This Module implements the TokenPrivileges class.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SECURITY_PRIVILEGE_HPP_
#define _WIN32EX_SECURITY_PRIVILEGE_HPP_

#include "../Internal/version.h"
#define WIN32EX_SECURITY_PRIVILEGE_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SECURITY_PRIVILEGE_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SECURITY_PRIVILEGE_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "../Internal/misc.hpp"
#include "Privilege.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <sddl.h>
#include <stdlib.h>

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <functional>
#include <vector>

namespace Win32Ex
{
inline bool operator==(const LUID &lhs, const LUID &rhs)
{
    return (lhs.LowPart == rhs.LowPart) && (lhs.HighPart == rhs.HighPart);
}

namespace Security
{
////////////////////////////////////////////////////////////////////////
//                                                                    //
//               NT Defined Privileges                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////
#ifdef UNICODE
#define SE_ASSIGNPRIMARYTOKEN_NAME_W SE_ASSIGNPRIMARYTOKEN_NAME
#define SE_INCREASE_QUOTA_NAME_W SE_INCREASE_QUOTA_NAME
#else
#define SE_ASSIGNPRIMARYTOKEN_NAME_W _W(SE_ASSIGNPRIMARYTOKEN_NAME)
#define SE_INCREASE_QUOTA_NAME_W _W(SE_INCREASE_QUOTA_NAME)
#endif

inline StringT ToPrivilegeName(LUID luid)
{
    DWORD length = 256;
    StringT name(length, _T('\0'));

    if (LookupPrivilegeName(NULL, &luid, &name[0], &length))
    {
        return name;
    }

    name.resize(length);
    if (LookupPrivilegeName(NULL, &luid, &name[0], &length))
    {
        return name;
    }
    return StringT();
}

inline LUID FromPrivilegeName(PCTSTR PrivilegeName)
{
    LUID luid;
    LookupPrivilegeValue(NULL, PrivilegeName, &luid);
    return luid;
}

inline std::vector<LUID> FromPrivilegeNames(const std::vector<LPCTSTR> &PrivilegeNames)
{
    std::vector<LUID> luids;
    for (std::vector<LPCTSTR>::const_iterator it = PrivilegeNames.begin(); it != PrivilegeNames.end(); ++it)
    {
        LUID luid;
        LookupPrivilegeValue(NULL, *it, &luid);
        luids.push_back(luid);
    }
    return luids;
}

DECLSPEC_SELECTANY LUID SeCreateTokenPrivilege = FromPrivilegeName(SE_CREATE_TOKEN_NAME);
DECLSPEC_SELECTANY LUID SeAssignPrimaryTokenPrivilege = FromPrivilegeName(SE_ASSIGNPRIMARYTOKEN_NAME);
DECLSPEC_SELECTANY LUID SeLockMemoryPrivilege = FromPrivilegeName(SE_LOCK_MEMORY_NAME);
DECLSPEC_SELECTANY LUID SeIncreaseQuotaPrivilege = FromPrivilegeName(SE_INCREASE_QUOTA_NAME);
DECLSPEC_SELECTANY LUID SeUnsolicitedInputPrivilege = FromPrivilegeName(SE_UNSOLICITED_INPUT_NAME);
DECLSPEC_SELECTANY LUID SeMachineAccountPrivilege = FromPrivilegeName(SE_MACHINE_ACCOUNT_NAME);
DECLSPEC_SELECTANY LUID SeTcbPrivilege = FromPrivilegeName(SE_TCB_NAME);
DECLSPEC_SELECTANY LUID SeSecurityPrivilege = FromPrivilegeName(SE_SECURITY_NAME);
DECLSPEC_SELECTANY LUID SeTakeOwnershipPrivilege = FromPrivilegeName(SE_TAKE_OWNERSHIP_NAME);
DECLSPEC_SELECTANY LUID SeLoadDriverPrivilege = FromPrivilegeName(SE_LOAD_DRIVER_NAME);
DECLSPEC_SELECTANY LUID SeSystemProfilePrivilege = FromPrivilegeName(SE_SYSTEM_PROFILE_NAME);
DECLSPEC_SELECTANY LUID SeSystemtimePrivilege = FromPrivilegeName(SE_SYSTEMTIME_NAME);
DECLSPEC_SELECTANY LUID SeProfileSingleProcessPrivilege = FromPrivilegeName(SE_PROF_SINGLE_PROCESS_NAME);
DECLSPEC_SELECTANY LUID SeIncreaseBasePriorityPrivilege = FromPrivilegeName(SE_INC_BASE_PRIORITY_NAME);
DECLSPEC_SELECTANY LUID SeCreatePagefilePrivilege = FromPrivilegeName(SE_CREATE_PAGEFILE_NAME);
DECLSPEC_SELECTANY LUID SeCreatePermanentPrivilege = FromPrivilegeName(SE_CREATE_PERMANENT_NAME);
DECLSPEC_SELECTANY LUID SeBackupPrivilege = FromPrivilegeName(SE_BACKUP_NAME);
DECLSPEC_SELECTANY LUID SeRestorePrivilege = FromPrivilegeName(SE_RESTORE_NAME);
DECLSPEC_SELECTANY LUID SeShutdownPrivilege = FromPrivilegeName(SE_SHUTDOWN_NAME);
DECLSPEC_SELECTANY LUID SeDebugPrivilege = FromPrivilegeName(SE_DEBUG_NAME);
DECLSPEC_SELECTANY LUID SeAuditPrivilege = FromPrivilegeName(SE_AUDIT_NAME);
DECLSPEC_SELECTANY LUID SeSystemEnvironmentPrivilege = FromPrivilegeName(SE_SYSTEM_ENVIRONMENT_NAME);
DECLSPEC_SELECTANY LUID SeChangeNotifyPrivilege = FromPrivilegeName(SE_CHANGE_NOTIFY_NAME);
DECLSPEC_SELECTANY LUID SeRemoteShutdownPrivilege = FromPrivilegeName(SE_REMOTE_SHUTDOWN_NAME);
DECLSPEC_SELECTANY LUID SeUndockPrivilege = FromPrivilegeName(SE_UNDOCK_NAME);
DECLSPEC_SELECTANY LUID SeSyncAgentPrivilege = FromPrivilegeName(SE_SYNC_AGENT_NAME);
DECLSPEC_SELECTANY LUID SeEnableDelegationPrivilege = FromPrivilegeName(SE_ENABLE_DELEGATION_NAME);
DECLSPEC_SELECTANY LUID SeManageVolumePrivilege = FromPrivilegeName(SE_MANAGE_VOLUME_NAME);
DECLSPEC_SELECTANY LUID SeImpersonatePrivilege = FromPrivilegeName(SE_IMPERSONATE_NAME);
DECLSPEC_SELECTANY LUID SeCreateGlobalPrivilege = FromPrivilegeName(SE_CREATE_GLOBAL_NAME);
DECLSPEC_SELECTANY LUID SeTrustedCredManAccessPrivilege = FromPrivilegeName(SE_TRUSTED_CREDMAN_ACCESS_NAME);
DECLSPEC_SELECTANY LUID SeRelabelPrivilege = FromPrivilegeName(SE_RELABEL_NAME);
DECLSPEC_SELECTANY LUID SeIncreaseWorkingSetPrivilege = FromPrivilegeName(SE_INC_WORKING_SET_NAME);
DECLSPEC_SELECTANY LUID SeTimeZonePrivilege = FromPrivilegeName(SE_TIME_ZONE_NAME);
DECLSPEC_SELECTANY LUID SeCreateSymbolicLinkPrivilege = FromPrivilegeName(SE_CREATE_SYMBOLIC_LINK_NAME);
#ifdef SE_DELEGATE_SESSION_USER_IMPERSONATE_NAME
DECLSPEC_SELECTANY LUID SeDelegateSessionUserImpersonatePrivilege =
    FromPrivilegeName(SE_DELEGATE_SESSION_USER_IMPERSONATE_NAME);
#endif

class TokenPrivileges
{
    WIN32EX_MOVE_ALWAYS_CLASS(TokenPrivileges)

  private:
    void Move(TokenPrivileges &To)
    {
        To.isPermanent_ = isPermanent_;
        To.enabled_ = enabled_;
        To.isAcquired_ = isAcquired_;
        To.previousPrivileges_ = previousPrivileges_;
        To.acquiredPrivileges_.swap(acquiredPrivileges_);

        isPermanent_ = false;
        enabled_ = false;
        isAcquired_ = false;
        ZeroMemory(&previousPrivileges_, sizeof(previousPrivileges_));
        acquiredPrivileges_.clear();
    }

  private:
    bool IsAcquired_(HANDLE TokenHandle, const std::vector<LUID> &Privileges)
    {
        if (!TokenHandle)
            TokenHandle = GetCurrentProcessToken();

        size_t matched = 0;
        PTOKEN_PRIVILEGES priv = GetTokenPrivileges(TokenHandle);
        if (priv)
        {
            for (std::vector<LUID>::const_iterator it = Privileges.begin(); it != Privileges.end(); ++it)
            {
                for (DWORD j = 0; j < priv->PrivilegeCount; ++j)
                {
                    const LUID &Privilege = *it;
                    if ((Privilege.HighPart == priv->Privileges[j].Luid.HighPart &&
                         Privilege.LowPart == priv->Privileges[j].Luid.LowPart) &&
                        (priv->Privileges[j].Attributes & SE_PRIVILEGE_ENABLED))
                    {
                        acquiredPrivileges_.push_back(ToPrivilegeName(Privilege));
                        ++matched;
                        break;
                    }
                }
            }

            FreeTokenPrivileges(priv);
        }
        return matched == Privileges.size();
    }

    bool IsAcquired_(HANDLE TokenHandle, size_t PrivilegeCount, const PCTSTR *PrivilegeNames)
    {
        std::vector<LUID> luids;
        LUID luid;
        for (size_t i = 0; i < PrivilegeCount; ++i)
        {
            if (LookupPrivilegeValue(NULL, PrivilegeNames[i], &luid))
                luids.push_back(luid);
        }
        return IsAcquired_(TokenHandle, luids);
    }

  public:
    TokenPrivileges(const LUID Privilege, HANDLE TokenHandle = NULL) : isPermanent_(false)
    {
        enabled_ = EnablePrivilegesEx(TRUE, 1, &Privilege, &previousPrivileges_, TokenHandle) == TRUE;
#ifdef __cpp_initializer_lists
        isAcquired_ = IsAcquired_(TokenHandle, {Privilege});
#else
        std::vector<LUID> Privileges;
        Privileges.push_back(Privilege);
        isAcquired_ = IsAcquired_(TokenHandle, Privileges);
#endif
    }

    TokenPrivileges(const PCTSTR Privilege, HANDLE TokenHandle = NULL) : isPermanent_(false)
    {
        enabled_ = EnablePrivileges(TRUE, 1, &Privilege, &previousPrivileges_, TokenHandle) == TRUE;

        isAcquired_ = IsAcquired_(TokenHandle, 1, &Privilege);
    }

    TokenPrivileges(const std::vector<LUID> &Privileges, HANDLE TokenHandle = NULL) : isPermanent_(false)
    {
        enabled_ = EnablePrivilegesEx(TRUE, (DWORD)Privileges.size(), (LUID *)&Privileges[0], &previousPrivileges_,
                                      TokenHandle) == TRUE;

        isAcquired_ = IsAcquired_(TokenHandle, Privileges);
    }

    TokenPrivileges(const std::vector<PCTSTR> &Privileges, HANDLE TokenHandle = NULL) : isPermanent_(false)
    {
        enabled_ = EnablePrivileges(TRUE, (DWORD)Privileges.size(), (PCTSTR *)&Privileges[0], &previousPrivileges_,
                                    TokenHandle) == TRUE;

        isAcquired_ = IsAcquired_(TokenHandle, Privileges.size(), (PCTSTR *)&Privileges[0]);
    }

    TokenPrivileges(HANDLE TokenHandle, DWORD PrivilegeCount, /* PrivilegeNames */...) : isPermanent_(false)
    {
        va_list PrivilegeNames;
        va_start(PrivilegeNames, PrivilegeCount);
        enabled_ = (EnablePrivileges(TRUE, PrivilegeCount, (PCTSTR *)PrivilegeNames, &previousPrivileges_,
                                     TokenHandle) == TRUE);
        isAcquired_ = IsAcquired_(TokenHandle, PrivilegeCount, (PCTSTR *)PrivilegeNames);
    }

    TokenPrivileges(DWORD PrivilegeCount, /* PrivilegeNames */...) : isPermanent_(false)
    {
        va_list PrivilegeNames;
        va_start(PrivilegeNames, PrivilegeCount);
        enabled_ =
            (EnablePrivileges(TRUE, PrivilegeCount, (PCTSTR *)PrivilegeNames, &previousPrivileges_, NULL) == TRUE);
        isAcquired_ = IsAcquired_(GetCurrentProcessToken(), PrivilegeCount, (PCTSTR *)PrivilegeNames);
    }

    ~TokenPrivileges()
    {
        if (!isPermanent_)
            Release();
    }

    void SetPermanent(bool IsPermanent)
    {
        isPermanent_ = IsPermanent;
    }

    void Release()
    {
        if (enabled_)
        {
            RevertPrivileges(&previousPrivileges_);
            enabled_ = false;
        }
    }

    bool IsAcquired() const
    {
        return isAcquired_;
    }

    const std::vector<StringT> AcquiredPrivileges() const
    {
        return acquiredPrivileges_;
    }

  private:
    bool isPermanent_;
    bool enabled_;
    bool isAcquired_;
    _PREVIOUS_TOKEN_PRIVILEGES previousPrivileges_;
    std::vector<StringT> acquiredPrivileges_;
};
} // namespace Security
} // namespace Win32Ex

#endif // _WIN32EX_SECURITY_PRIVILEGE_HPP_