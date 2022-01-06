/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    winsvc.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_TMPL_API_WINSVC_HPP_
#define _WIN32EX_TMPL_API_WINSVC_HPP_

#include "../Internal/version.h"
#define WIN32EX_TMPL_API_WINSVC_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_TMPL_API_WINSVC_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_TMPL_API_WINSVC_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "macros.hpp"

#include <winsvc.h>

namespace Win32Ex
{
WIN32EX_API_DEFINE_STRUCT_T(QUERY_SERVICE_CONFIG);
WIN32EX_API_DEFINE_STRUCT_T(SERVICE_DESCRIPTION);
WIN32EX_API_DEFINE_STRUCT_T(SERVICE_FAILURE_ACTIONS);
WIN32EX_API_DEFINE_STRUCT_T(SERVICE_REQUIRED_PRIVILEGES_INFO);
WIN32EX_API_DEFINE_STRUCT_T(SERVICE_TABLE_ENTRY);
WIN32EX_API_DEFINE_STRUCT_T(ENUM_SERVICE_STATUS);
WIN32EX_API_DEFINE_STRUCT_T(ENUM_SERVICE_STATUS_PROCESS);

#ifdef SERVICE_MAIN_FUNCTION
#undef SERVICE_MAIN_FUNCTION
WIN32EX_API_DEFINE_FN_T(SERVICE_MAIN_FUNCTION);
#ifdef UNICODE
#define SERVICE_MAIN_FUNCTION SERVICE_MAIN_FUNCTIONW
#else
#define SERVICE_MAIN_FUNCTION SERVICE_MAIN_FUNCTIONA
#endif // UNICODE
#endif

#ifdef LPSERVICE_MAIN_FUNCTION
#undef LPSERVICE_MAIN_FUNCTION
WIN32EX_API_DEFINE_FN_T(LPSERVICE_MAIN_FUNCTION);
#ifdef UNICODE
#define LPSERVICE_MAIN_FUNCTION LPSERVICE_MAIN_FUNCTIONW
#else
#define LPSERVICE_MAIN_FUNCTION LPSERVICE_MAIN_FUNCTIONA
#endif // UNICODE
#endif

template <typename _CharType>
inline SC_HANDLE WINAPI OpenSCManagerT(_In_opt_ CONST _CharType *lpMachineName,
                                       _In_opt_ CONST _CharType *lpDatabaseName, _In_ DWORD dwDesiredAccess);

template <>
inline SC_HANDLE WINAPI OpenSCManagerT<CHAR>(_In_opt_ LPCSTR lpMachineName, _In_opt_ LPCSTR lpDatabaseName,
                                             _In_ DWORD dwDesiredAccess)
{
    return OpenSCManagerA(lpMachineName, lpDatabaseName, dwDesiredAccess);
}

template <>
inline SC_HANDLE WINAPI OpenSCManagerT<WCHAR>(_In_opt_ LPCWSTR lpMachineName, _In_opt_ LPCWSTR lpDatabaseName,
                                              _In_ DWORD dwDesiredAccess)
{
    return OpenSCManagerW(lpMachineName, lpDatabaseName, dwDesiredAccess);
}

template <typename _CharType>
inline SC_HANDLE WINAPI CreateServiceT(_In_ SC_HANDLE hSCManager, _In_ const _CharType *lpServiceName,
                                       _In_opt_ const _CharType *lpDisplayName, _In_ DWORD dwDesiredAccess,
                                       _In_ DWORD dwServiceType, _In_ DWORD dwStartType, _In_ DWORD dwErrorControl,
                                       _In_opt_ const _CharType *lpBinaryPathName,
                                       _In_opt_ const _CharType *lpLoadOrderGroup, _Out_opt_ LPDWORD lpdwTagId,
                                       _In_opt_ const _CharType *lpDependencies,
                                       _In_opt_ const _CharType *lpServiceStartName,
                                       _In_opt_ const _CharType *lpPassword);

template <>
inline SC_HANDLE WINAPI CreateServiceT<CHAR>(_In_ SC_HANDLE hSCManager, _In_ LPCSTR lpServiceName,
                                             _In_opt_ LPCSTR lpDisplayName, _In_ DWORD dwDesiredAccess,
                                             _In_ DWORD dwServiceType, _In_ DWORD dwStartType,
                                             _In_ DWORD dwErrorControl, _In_opt_ LPCSTR lpBinaryPathName,
                                             _In_opt_ LPCSTR lpLoadOrderGroup, _Out_opt_ LPDWORD lpdwTagId,
                                             _In_opt_ LPCSTR lpDependencies, _In_opt_ LPCSTR lpServiceStartName,
                                             _In_opt_ LPCSTR lpPassword)
{
    return CreateServiceA(hSCManager, lpServiceName, lpDisplayName, dwDesiredAccess, dwServiceType, dwStartType,
                          dwErrorControl, lpBinaryPathName, lpLoadOrderGroup, lpdwTagId, lpDependencies,
                          lpServiceStartName, lpPassword);
}

template <>
inline SC_HANDLE WINAPI CreateServiceT<WCHAR>(_In_ SC_HANDLE hSCManager, _In_ LPCWSTR lpServiceName,
                                              _In_opt_ LPCWSTR lpDisplayName, _In_ DWORD dwDesiredAccess,
                                              _In_ DWORD dwServiceType, _In_ DWORD dwStartType,
                                              _In_ DWORD dwErrorControl, _In_opt_ LPCWSTR lpBinaryPathName,
                                              _In_opt_ LPCWSTR lpLoadOrderGroup, _Out_opt_ LPDWORD lpdwTagId,
                                              _In_opt_ LPCWSTR lpDependencies, _In_opt_ LPCWSTR lpServiceStartName,
                                              _In_opt_ LPCWSTR lpPassword)
{
    return CreateServiceW(hSCManager, lpServiceName, lpDisplayName, dwDesiredAccess, dwServiceType, dwStartType,
                          dwErrorControl, lpBinaryPathName, lpLoadOrderGroup, lpdwTagId, lpDependencies,
                          lpServiceStartName, lpPassword);
}

template <typename _CharType>
inline SC_HANDLE WINAPI OpenServiceT(_In_ SC_HANDLE hSCManager, _In_ const _CharType *lpServiceName,
                                     _In_ DWORD dwDesiredAccess);

template <>
inline SC_HANDLE WINAPI OpenServiceT<CHAR>(_In_ SC_HANDLE hSCManager, _In_ PCSTR lpServiceName,
                                           _In_ DWORD dwDesiredAccess)
{
    return OpenServiceA(hSCManager, lpServiceName, dwDesiredAccess);
}

template <>
inline SC_HANDLE WINAPI OpenServiceT<WCHAR>(_In_ SC_HANDLE hSCManager, _In_ PCWSTR lpServiceName,
                                            _In_ DWORD dwDesiredAccess)
{
    return OpenServiceW(hSCManager, lpServiceName, dwDesiredAccess);
}

template <typename _CharType>
inline BOOL WINAPI QueryServiceConfig2T(_In_ SC_HANDLE hService, _In_ DWORD dwInfoLevel, _Out_ LPBYTE lpBuffer,
                                        _In_ DWORD cbBufSize, _Out_ LPDWORD pcbBytesNeeded);
template <>
inline BOOL WINAPI QueryServiceConfig2T<CHAR>(_In_ SC_HANDLE hService, _In_ DWORD dwInfoLevel, _Out_ LPBYTE lpBuffer,
                                              _In_ DWORD cbBufSize, _Out_ LPDWORD pcbBytesNeeded)
{
    return QueryServiceConfig2A(hService, dwInfoLevel, lpBuffer, cbBufSize, pcbBytesNeeded);
}

template <>
inline BOOL WINAPI QueryServiceConfig2T<WCHAR>(_In_ SC_HANDLE hService, _In_ DWORD dwInfoLevel, _Out_ LPBYTE lpBuffer,
                                               _In_ DWORD cbBufSize, _Out_ LPDWORD pcbBytesNeeded)
{
    return QueryServiceConfig2W(hService, dwInfoLevel, lpBuffer, cbBufSize, pcbBytesNeeded);
}

template <typename _CharType>
inline BOOL WINAPI ChangeServiceConfig2T(_In_ SC_HANDLE hService, _In_ DWORD dwInfoLevel, _In_opt_ LPVOID lpInfo);

template <>
inline BOOL WINAPI ChangeServiceConfig2T<CHAR>(_In_ SC_HANDLE hService, _In_ DWORD dwInfoLevel, _In_opt_ LPVOID lpInfo)
{
    return ChangeServiceConfig2A(hService, dwInfoLevel, lpInfo);
}

template <>
inline BOOL WINAPI ChangeServiceConfig2T<WCHAR>(_In_ SC_HANDLE hService, _In_ DWORD dwInfoLevel, _In_opt_ LPVOID lpInfo)
{
    return ChangeServiceConfig2W(hService, dwInfoLevel, lpInfo);
}

template <typename _CharType>
inline BOOL WINAPI QueryServiceConfigT(_In_ SC_HANDLE hService, _Out_ QUERY_SERVICE_CONFIGT<_CharType> *lpServiceConfig,
                                       _In_ DWORD cbBufSize, _Out_ LPDWORD pcbBytesNeeded);

template <>
inline BOOL WINAPI QueryServiceConfigT<CHAR>(_In_ SC_HANDLE hService,
                                             _Out_ QUERY_SERVICE_CONFIGT<CHAR> *lpServiceConfig, _In_ DWORD cbBufSize,
                                             _Out_ LPDWORD pcbBytesNeeded)
{
    return QueryServiceConfigA(hService, lpServiceConfig, cbBufSize, pcbBytesNeeded);
}

template <>
inline BOOL WINAPI QueryServiceConfigT<WCHAR>(_In_ SC_HANDLE hService,
                                              _Out_ QUERY_SERVICE_CONFIGT<WCHAR> *lpServiceConfig, _In_ DWORD cbBufSize,
                                              _Out_ LPDWORD pcbBytesNeeded)
{
    return QueryServiceConfigW(hService, lpServiceConfig, cbBufSize, pcbBytesNeeded);
}

template <typename _CharType>
inline BOOL WINAPI EnumServicesStatusExT(_In_ SC_HANDLE hSCManager, _In_ SC_ENUM_TYPE InfoLevel,
                                         _In_ DWORD dwServiceType, _In_ DWORD dwServiceState, _Out_ LPBYTE lpServices,
                                         _In_ DWORD cbBufSize, _Out_ LPDWORD pcbBytesNeeded,
                                         _Out_ LPDWORD lpServicesReturned, _Inout_opt_ LPDWORD lpResumeHandle,
                                         _In_opt_ CONST _CharType *pszGroupName);

template <>
inline BOOL WINAPI EnumServicesStatusExT<CHAR>(_In_ SC_HANDLE hSCManager, _In_ SC_ENUM_TYPE InfoLevel,
                                               _In_ DWORD dwServiceType, _In_ DWORD dwServiceState,
                                               _Out_ LPBYTE lpServices, _In_ DWORD cbBufSize,
                                               _Out_ LPDWORD pcbBytesNeeded, _Out_ LPDWORD lpServicesReturned,
                                               _Inout_opt_ LPDWORD lpResumeHandle, _In_opt_ LPCSTR pszGroupName)
{
    return EnumServicesStatusExA(hSCManager, InfoLevel, dwServiceType, dwServiceState, lpServices, cbBufSize,
                                 pcbBytesNeeded, lpServicesReturned, lpResumeHandle, pszGroupName);
}

template <>
inline BOOL WINAPI EnumServicesStatusExT<WCHAR>(_In_ SC_HANDLE hSCManager, _In_ SC_ENUM_TYPE InfoLevel,
                                                _In_ DWORD dwServiceType, _In_ DWORD dwServiceState,
                                                _Out_ LPBYTE lpServices, _In_ DWORD cbBufSize,
                                                _Out_ LPDWORD pcbBytesNeeded, _Out_ LPDWORD lpServicesReturned,
                                                _Inout_opt_ LPDWORD lpResumeHandle, _In_opt_ LPCWSTR pszGroupName)
{
    return EnumServicesStatusExW(hSCManager, InfoLevel, dwServiceType, dwServiceState, lpServices, cbBufSize,
                                 pcbBytesNeeded, lpServicesReturned, lpResumeHandle, pszGroupName);
}

template <typename _CharType>
inline BOOL WINAPI EnumDependentServicesT(_In_ SC_HANDLE hService, _In_ DWORD dwServiceState,
                                          _Out_ ENUM_SERVICE_STATUST<_CharType> *lpServices, _In_ DWORD cbBufSize,
                                          _Out_ LPDWORD pcbBytesNeeded, _Out_ LPDWORD lpServicesReturned);
template <>
inline BOOL WINAPI EnumDependentServicesT<CHAR>(_In_ SC_HANDLE hService, _In_ DWORD dwServiceState,
                                                _Out_ ENUM_SERVICE_STATUST<CHAR> *lpServices, _In_ DWORD cbBufSize,
                                                _Out_ LPDWORD pcbBytesNeeded, _Out_ LPDWORD lpServicesReturned)
{
    return EnumDependentServicesA(hService, dwServiceState, lpServices, cbBufSize, pcbBytesNeeded, lpServicesReturned);
}

template <>
inline BOOL WINAPI EnumDependentServicesT<WCHAR>(_In_ SC_HANDLE hService, _In_ DWORD dwServiceState,
                                                 _Out_ ENUM_SERVICE_STATUST<WCHAR> *lpServices, _In_ DWORD cbBufSize,
                                                 _Out_ LPDWORD pcbBytesNeeded, _Out_ LPDWORD lpServicesReturned)
{
    return EnumDependentServicesW(hService, dwServiceState, lpServices, cbBufSize, pcbBytesNeeded, lpServicesReturned);
}

template <typename _CharType>
inline BOOL WINAPI StartServiceCtrlDispatcherT(_In_ CONST
                                               typename SERVICE_TABLE_ENTRYT<_CharType>::Type *lpServiceStartTable);

template <>
inline BOOL WINAPI StartServiceCtrlDispatcherT<CHAR>(_In_ CONST
                                                     typename SERVICE_TABLE_ENTRYT<CHAR>::Type *lpServiceStartTable)
{
    return StartServiceCtrlDispatcherA(lpServiceStartTable);
}

template <>
inline BOOL WINAPI StartServiceCtrlDispatcherT<WCHAR>(_In_ CONST
                                                      typename SERVICE_TABLE_ENTRYT<WCHAR>::Type *lpServiceStartTable)
{
    return StartServiceCtrlDispatcherW(lpServiceStartTable);
}

template <typename _CharType>
inline SERVICE_STATUS_HANDLE WINAPI RegisterServiceCtrlHandlerExT(_In_ CONST _CharType *lpServiceName,
                                                                  _In_ LPHANDLER_FUNCTION_EX lpHandlerProc,
                                                                  _In_opt_ LPVOID lpContext);
template <>
inline SERVICE_STATUS_HANDLE WINAPI RegisterServiceCtrlHandlerExT<CHAR>(_In_ LPCSTR lpServiceName,
                                                                        _In_ LPHANDLER_FUNCTION_EX lpHandlerProc,
                                                                        _In_opt_ LPVOID lpContext)
{
    return RegisterServiceCtrlHandlerExA(lpServiceName, lpHandlerProc, lpContext);
}
template <>
inline SERVICE_STATUS_HANDLE WINAPI RegisterServiceCtrlHandlerExT<WCHAR>(_In_ LPCWSTR lpServiceName,
                                                                         _In_ LPHANDLER_FUNCTION_EX lpHandlerProc,
                                                                         _In_opt_ LPVOID lpContext)
{
    return RegisterServiceCtrlHandlerExW(lpServiceName, lpHandlerProc, lpContext);
}
} // namespace Win32Ex

#endif // WIN32EX_TMPL_API_WINSVC_HPP
