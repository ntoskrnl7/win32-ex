/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Sid.h

Abstract:

    This Module implements the SID variables and procedures.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <AclAPI.h>

DECLSPEC_SELECTANY SID EveryoneSid = {SID_REVISION, 1, SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID};

#ifdef SECURITY_LOCAL_LOGON_RID
DECLSPEC_SELECTANY SID ConsoleLogonSid = {SID_REVISION, 1, SECURITY_LOCAL_SID_AUTHORITY, SECURITY_LOCAL_LOGON_RID};
#endif
DECLSPEC_SELECTANY SID LocalSid = {SID_REVISION, 1, SECURITY_LOCAL_SID_AUTHORITY, SECURITY_LOCAL_RID};

DECLSPEC_SELECTANY SID CreatorOwnerSid = {SID_REVISION, 1, SECURITY_CREATOR_SID_AUTHORITY, SECURITY_CREATOR_OWNER_RID};
DECLSPEC_SELECTANY SID CreatorGroupSid = {SID_REVISION, 1, SECURITY_CREATOR_SID_AUTHORITY, SECURITY_CREATOR_GROUP_RID};

DECLSPEC_SELECTANY SID ServiceSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_SERVICE_RID};
DECLSPEC_SELECTANY SID LocalSystemSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_LOCAL_SYSTEM_RID};
DECLSPEC_SELECTANY SID LocalServiceSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_LOCAL_SERVICE_RID};
DECLSPEC_SELECTANY SID NetworkServiceSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_NETWORK_SERVICE_RID};

FORCEINLINE PSID GetAdministratorsSid()
{
    PSID sid;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0,
                                  0, &sid))
    {
        return NULL;
    }

    return sid;
}

FORCEINLINE VOID FreeLogonSid(PSID *ppsid)
{
    HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
}

FORCEINLINE BOOL GetLogonSid(HANDLE hToken, PSID *ppsid, PDWORD pdwsidLength)
{
    BOOL bSuccess = FALSE;
    DWORD dwIndex;
    DWORD dwLength = 0;
    PTOKEN_GROUPS ptg = NULL;

    // Verify the parameter passed in is not NULL.
    if (NULL == ppsid)
        goto Cleanup;

    // Get required buffer size and allocate the TOKEN_GROUPS buffer.

    if (!GetTokenInformation(hToken,      // handle to the access token
                             TokenGroups, // get information about the token's groups
                             (LPVOID)ptg, // pointer to TOKEN_GROUPS buffer
                             0,           // size of buffer
                             &dwLength    // receives required buffer size
                             ))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            goto Cleanup;

        ptg = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength);

        if (ptg == NULL)
            goto Cleanup;
    }

    // Get the token group information from the access token.

    if (!GetTokenInformation(hToken,      // handle to the access token
                             TokenGroups, // get information about the token's groups
                             (LPVOID)ptg, // pointer to TOKEN_GROUPS buffer
                             dwLength,    // size of buffer
                             &dwLength    // receives required buffer size
                             ))
    {
        goto Cleanup;
    }

    if (ptg == NULL)
        goto Cleanup;

    // Loop through the groups to find the logon SID.

    for (dwIndex = 0; dwIndex < ptg->GroupCount; dwIndex++)
        if ((ptg->Groups[dwIndex].Attributes & SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID)
        {
            // Found the logon SID; make a copy of it.

            dwLength = GetLengthSid(ptg->Groups[dwIndex].Sid);
            *pdwsidLength = dwLength;
            *ppsid = (PSID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength);
            if (*ppsid == NULL)
                goto Cleanup;
            if (!CopySid(dwLength, *ppsid, ptg->Groups[dwIndex].Sid))
            {
                HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
                goto Cleanup;
            }
            break;
        }

    bSuccess = TRUE;

Cleanup:

    // Free the buffer for the token groups.

    if (ptg != NULL)
        HeapFree(GetProcessHeap(), 0, (LPVOID)ptg);

    return bSuccess;
}