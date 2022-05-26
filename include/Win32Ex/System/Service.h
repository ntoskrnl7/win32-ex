/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Service.h

Abstract:

    This Module implements the Service functions.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SYSTEM_SERVICE_H_
#define _WIN32EX_SYSTEM_SERVICE_H_

#include "../Internal/version.h"
#define WIN32EX_SYSTEM_SERVICE_H_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SYSTEM_SERVICE_H_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SYSTEM_SERVICE_H_VERSION_PATCH WIN32EX_VERSION_PATCH

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <stdlib.h>

#include "../Internal/misc.h"

WIN32EX_ALWAYS_INLINE LPENUM_SERVICE_STATUS_PROCESSW LookupServiceStatusProcessByProcessIdW(_In_ DWORD dwProcessId,
                                                                                       _In_ DWORD dwServiceType,
                                                                                       _In_ DWORD dwServiceState)
{
    SC_HANDLE hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (hSCManager == NULL)
        return NULL;

    DWORD bytesNeeded;
    DWORD serviceCount = 0;
    EnumServicesStatusExW(hSCManager, SC_ENUM_PROCESS_INFO, dwServiceType, dwServiceState, NULL, 0, &bytesNeeded,
                          &serviceCount, NULL, NULL);

    LPENUM_SERVICE_STATUS_PROCESSW statusProcess = (LPENUM_SERVICE_STATUS_PROCESSW)malloc(bytesNeeded);
    if (EnumServicesStatusExW(hSCManager, SC_ENUM_PROCESS_INFO, dwServiceType, dwServiceState, (LPBYTE)statusProcess,
                              bytesNeeded, &bytesNeeded, &serviceCount, NULL, NULL) == FALSE)
    {
        CloseServiceHandle(hSCManager);
        return NULL;
    }
    CloseServiceHandle(hSCManager);

    for (DWORD i = 0; i < serviceCount; ++i)
    {
        if (statusProcess[i].ServiceStatusProcess.dwProcessId == dwProcessId)
        {
            LPENUM_SERVICE_STATUS_PROCESSW result =
                (LPENUM_SERVICE_STATUS_PROCESSW)malloc(sizeof(ENUM_SERVICE_STATUS_PROCESSW));
            if (result == NULL)
            {
                free(statusProcess);
                return NULL;
            }
#ifdef _MSC_VER
            result->lpDisplayName = _wcsdup(statusProcess[i].lpDisplayName);
            result->lpServiceName = _wcsdup(statusProcess[i].lpServiceName);
#else
            result->lpDisplayName = wcsdup(statusProcess[i].lpDisplayName);
            result->lpServiceName = wcsdup(statusProcess[i].lpServiceName);
#endif
            result->ServiceStatusProcess = statusProcess[i].ServiceStatusProcess;
            return result;
        }
    }
    return NULL;
}

WIN32EX_ALWAYS_INLINE VOID ReleaseServiceStatusProcess(_In_ LPENUM_SERVICE_STATUS_PROCESSW lpStatus)
{
    if (lpStatus == NULL)
        return;
    if (lpStatus->lpDisplayName)
        free(lpStatus->lpDisplayName);
    if (lpStatus->lpServiceName)
        free(lpStatus->lpServiceName);
    free(lpStatus);
}
#endif // _WIN32EX_SYSTEM_SERVICE_H_