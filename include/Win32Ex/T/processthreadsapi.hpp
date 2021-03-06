/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    processthreadsapi.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_TMPL_API_PROCESSTHREADSAPI_HPP_
#define _WIN32EX_TMPL_API_PROCESSTHREADSAPI_HPP_

#include "../Internal/version.h"
#define WIN32EX_TMPL_API_PROCESSTHREADSAPI_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_TMPL_API_PROCESSTHREADSAPI_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_TMPL_API_PROCESSTHREADSAPI_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "macros.hpp"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

namespace Win32Ex
{
WIN32EX_API_DEFINE_STRUCT_T(STARTUPINFO);

WIN32EX_API_DEFINE_STRUCT_T(STARTUPINFOEX);

template <typename CharType>
inline BOOL QueryFullProcessImageNameT(_In_ HANDLE hProcess, _In_ DWORD dwFlags, _Out_ CharType *lpExeName,
                                       _Inout_ PDWORD lpdwSize);

template <>
inline BOOL QueryFullProcessImageNameT<CHAR>(_In_ HANDLE hProcess, _In_ DWORD dwFlags, _Out_ LPSTR lpExeName,
                                             _Inout_ PDWORD lpdwSize)
{
    return QueryFullProcessImageNameA(hProcess, dwFlags, lpExeName, lpdwSize);
}
template <>
inline BOOL QueryFullProcessImageNameT<WCHAR>(_In_ HANDLE hProcess, _In_ DWORD dwFlags, _Out_ LPWSTR lpExeName,
                                              _Inout_ PDWORD lpdwSize)
{
    return QueryFullProcessImageNameW(hProcess, dwFlags, lpExeName, lpdwSize);
}

template <typename CharType>
inline BOOL CreateProcessT(_In_opt_ const CharType *lpApplicationName, _Inout_opt_ CharType *lpCommandLine,
                           _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                           _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ BOOL bInheritHandles,
                           _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
                           _In_opt_ const CharType *lpCurrentDirectory,
                           _In_ typename Win32Ex::STARTUPINFOT<CharType>::Type *lpStartupInfo,
                           _Out_ LPPROCESS_INFORMATION lpProcessInformation);

template <>
inline BOOL CreateProcessT<CHAR>(_In_opt_ LPCSTR lpApplicationName, _Inout_opt_ LPSTR lpCommandLine,
                                 _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                 _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ BOOL bInheritHandles,
                                 _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
                                 _In_opt_ LPCSTR lpCurrentDirectory, _In_ LPSTARTUPINFOA lpStartupInfo,
                                 _Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
    return ::CreateProcessA(
        lpApplicationName, lpCommandLine, reinterpret_cast<::_SECURITY_ATTRIBUTES *>(lpProcessAttributes),
        reinterpret_cast<::_SECURITY_ATTRIBUTES *>(lpThreadAttributes), bInheritHandles, dwCreationFlags, lpEnvironment,
        lpCurrentDirectory, reinterpret_cast<::_STARTUPINFOA *>(lpStartupInfo),
        reinterpret_cast<::_PROCESS_INFORMATION *>(lpProcessInformation));
}
template <>
inline BOOL CreateProcessT<WCHAR>(_In_opt_ LPCWSTR lpApplicationName, _Inout_opt_ LPWSTR lpCommandLine,
                                  _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                  _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ BOOL bInheritHandles,
                                  _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
                                  _In_opt_ LPCWSTR lpCurrentDirectory, _In_ LPSTARTUPINFOW lpStartupInfo,
                                  _Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
    return ::CreateProcessW(
        lpApplicationName, lpCommandLine, reinterpret_cast<::_SECURITY_ATTRIBUTES *>(lpProcessAttributes),
        reinterpret_cast<::_SECURITY_ATTRIBUTES *>(lpThreadAttributes), bInheritHandles, dwCreationFlags, lpEnvironment,
#if defined(_WIN32_WCE)
        const_cast<LPWSTR_>(lpCurrentDirectory),
#else
        lpCurrentDirectory,
#endif
        reinterpret_cast<::_STARTUPINFOW *>(lpStartupInfo),
        reinterpret_cast<::_PROCESS_INFORMATION *>(lpProcessInformation));
}

template <typename CharType>
inline BOOL CreateProcessAsUserT(_In_opt_ HANDLE hToken, _In_opt_ const CharType *lpApplicationName,
                                 _Inout_opt_ CharType *lpCommandLine,
                                 _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                 _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ BOOL bInheritHandles,
                                 _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
                                 _In_opt_ const CharType *lpCurrentDirectory,
                                 _In_ typename Win32Ex::STARTUPINFOT<CharType>::Type *lpStartupInfo,
                                 _Out_ LPPROCESS_INFORMATION lpProcessInformation);

template <>
inline BOOL CreateProcessAsUserT<CHAR>(_In_opt_ HANDLE hToken, _In_opt_ LPCSTR lpApplicationName,
                                       _Inout_opt_ LPSTR lpCommandLine,
                                       _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                       _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ BOOL bInheritHandles,
                                       _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
                                       _In_opt_ LPCSTR lpCurrentDirectory, _In_ LPSTARTUPINFOA lpStartupInfo,
                                       _Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
    return ::CreateProcessAsUserA(
        hToken, lpApplicationName, lpCommandLine, reinterpret_cast<::_SECURITY_ATTRIBUTES *>(lpProcessAttributes),
        reinterpret_cast<::_SECURITY_ATTRIBUTES *>(lpThreadAttributes), bInheritHandles, dwCreationFlags, lpEnvironment,
        lpCurrentDirectory, reinterpret_cast<::_STARTUPINFOA *>(lpStartupInfo),
        reinterpret_cast<::_PROCESS_INFORMATION *>(lpProcessInformation));
}

template <>
inline BOOL CreateProcessAsUserT<WCHAR>(_In_opt_ HANDLE hToken, _In_opt_ LPCWSTR lpApplicationName,
                                        _Inout_opt_ LPWSTR lpCommandLine,
                                        _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                        _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ BOOL bInheritHandles,
                                        _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
                                        _In_opt_ LPCWSTR lpCurrentDirectory, _In_ LPSTARTUPINFOW lpStartupInfo,
                                        _Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
    return ::CreateProcessAsUserW(
        hToken, lpApplicationName, lpCommandLine, reinterpret_cast<::_SECURITY_ATTRIBUTES *>(lpProcessAttributes),
        reinterpret_cast<::_SECURITY_ATTRIBUTES *>(lpThreadAttributes), bInheritHandles, dwCreationFlags, lpEnvironment,
#if defined(_WIN32_WCE)
        const_cast<LPWSTR_>(lpCurrentDirectory),
#else
        lpCurrentDirectory,
#endif
        reinterpret_cast<::_STARTUPINFOW *>(lpStartupInfo),
        reinterpret_cast<::_PROCESS_INFORMATION *>(lpProcessInformation));
}
} // namespace Win32Ex

#endif // _WIN32EX_TMPL_API_PROCESSTHREADSAPI_HPP_
