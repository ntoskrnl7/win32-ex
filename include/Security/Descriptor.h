/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Descriptor.h

Abstract:

    This Module implements the security descriptor procedures.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "..\Internal\misc.h"
#include ".\Sid.h"

inline void FreeSecurityDescriptor(PSECURITY_DESCRIPTOR SecurityDescriptor)
{
    HeapFree(GetProcessHeap(), 0, SecurityDescriptor);
}

inline PSECURITY_DESCRIPTOR AllocateAndInitializeSecurityDescriptor(
    DWORD SystemDesireAccess, DWORD AdministratorsDesireAccess, DWORD LogonUserDesireAccess,
    DWORD OtherUserDesireAccess, DWORD OwnerDesireAccess, DWORD GroupDesireAccess, DWORD EveryoneDesireAccess)
{
    DWORD result;
    PSECURITY_DESCRIPTOR securityDescriptor = NULL;
    ULONG securityDescriptorLength;

    DWORD systemSidLength = sizeof(SID);

    PSID administratorsSid = NULL;
    DWORD administratorsSidLength;

    PSID logonSid = NULL;
    DWORD logonSidLength = 0;
    DWORD consoleLogonSidLength = sizeof(SID);

    DWORD everyoneSidLength = sizeof(SID);
    DWORD ownerSidLength = sizeof(SID);
    DWORD groupSidLength = sizeof(SID);

    PACL acl;
    DWORD aclLength;

    HANDLE tokenHandle;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
    {
        if (!GetLogonSid(tokenHandle, &logonSid, &logonSidLength))
        {
            logonSid = NULL;
        }
        CloseHandle(tokenHandle);
    }

    administratorsSid = GetAdministratorsSid();
    if (!administratorsSid)
    {
        result = GetLastError();
        goto Cleanup;
    }

    administratorsSidLength = GetLengthSid(administratorsSid);

    aclLength = sizeof(ACL) + (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + systemSidLength) +
                (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + administratorsSidLength) +
                (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + ownerSidLength) +
                (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + groupSidLength) +
                (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + consoleLogonSidLength) +
                (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + everyoneSidLength);

    if (logonSid)
    {
        aclLength += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + logonSidLength);
    }

    securityDescriptorLength = SECURITY_DESCRIPTOR_MIN_LENGTH + aclLength;
    securityDescriptor = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, securityDescriptorLength);
    if (!securityDescriptor)
    {
        result = GetLastError();
        goto Cleanup;
    }

    acl = (PACL)Add2Ptr(securityDescriptor, SECURITY_DESCRIPTOR_MIN_LENGTH);

    if (!InitializeAcl(acl, aclLength, ACL_REVISION2))
    {
        result = GetLastError();
        goto Cleanup;
    }

    if (!InitializeSecurityDescriptor(securityDescriptor, SECURITY_DESCRIPTOR_REVISION1))
    {
        result = GetLastError();
        goto Cleanup;
    }

    if (!AddAccessAllowedAce(acl, ACL_REVISION2, SystemDesireAccess, &LocalSystemSid))
    {
        result = GetLastError();
        goto Cleanup;
    }

    if (!AddAccessAllowedAce(acl, ACL_REVISION2, AdministratorsDesireAccess, administratorsSid))
    {
        result = GetLastError();
        goto Cleanup;
    }

    if (logonSid)
    {
        if (!AddAccessAllowedAce(acl, ACL_REVISION2, LogonUserDesireAccess, logonSid))
        {
            result = GetLastError();
            goto Cleanup;
        }
    }

    if (OtherUserDesireAccess)
    {
#ifdef SECURITY_LOCAL_LOGON_RID
        if (!AddAccessAllowedAce(acl, ACL_REVISION2, OtherUserDesireAccess, &ConsoleLogonSid))
#else
        if (!AddAccessAllowedAce(acl, ACL_REVISION2, OtherUserDesireAccess, &LocalSid))
#endif
        {
            result = GetLastError();
            goto Cleanup;
        }
    }

    if (OwnerDesireAccess)
    {
        if (!AddAccessAllowedAce(acl, ACL_REVISION2, OwnerDesireAccess, &CreatorOwnerSid))
        {
            result = GetLastError();
            goto Cleanup;
        }
    }

    if (GroupDesireAccess)
    {
        if (!AddAccessAllowedAce(acl, ACL_REVISION2, GroupDesireAccess, &CreatorGroupSid))
        {
            result = GetLastError();
            goto Cleanup;
        }
    }

    if (EveryoneDesireAccess)
    {
        if (!AddAccessAllowedAce(acl, ACL_REVISION2, EveryoneDesireAccess, &EveryoneSid))
        {
            result = GetLastError();
            goto Cleanup;
        }
    }

    if (!SetSecurityDescriptorDacl(securityDescriptor, TRUE, acl, FALSE))
    {
        result = GetLastError();
        goto Cleanup;
    }

    result = ERROR_SUCCESS;

Cleanup:

    if (logonSid)
    {
        FreeLogonSid(&logonSid);
    }

    if (administratorsSid)
    {
        FreeSid(administratorsSid);
    }

    if (result != ERROR_SUCCESS)
    {
        FreeSecurityDescriptor(securityDescriptor);
    }

    return securityDescriptor;
}
