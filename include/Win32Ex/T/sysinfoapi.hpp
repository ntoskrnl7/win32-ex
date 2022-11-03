/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    sysinfoapi.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_TMPL_API_SYSINFOAPI_HPP_
#define _WIN32EX_TMPL_API_SYSINFOAPI_HPP_

#include "../Internal/version.h"
#define WIN32EX_TMPL_API_SYSINFOAPI_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_TMPL_API_SYSINFOAPI_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_TMPL_API_SYSINFOAPI_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

namespace Win32Ex
{
template <typename CharType> inline DWORD GetSystemDirectoryT(_Out_ CharType *lpBuffer, _In_ UINT uSize);

template <> inline DWORD GetSystemDirectoryT<CHAR>(_Out_ LPSTR lpBuffer, _In_ UINT uSize)
{
    return GetSystemDirectoryA(lpBuffer, uSize);
}

template <> inline DWORD GetSystemDirectoryT<WCHAR>(_Out_ LPWSTR lpBuffer, _In_ UINT uSize)
{
    return GetSystemDirectoryW(lpBuffer, uSize);
}

template <typename CharType> inline DWORD GetWindowsDirectoryT(_Out_ CharType *lpBuffer, _In_ UINT uSize);

template <> inline DWORD GetWindowsDirectoryT<CHAR>(_Out_ LPSTR lpBuffer, _In_ UINT uSize)
{
    return GetWindowsDirectoryA(lpBuffer, uSize);
}

template <> inline DWORD GetWindowsDirectoryT<WCHAR>(_Out_ LPWSTR lpBuffer, _In_ UINT uSize)
{
    return GetWindowsDirectoryW(lpBuffer, uSize);
}
} // namespace Win32Ex

#endif // _WIN32EX_TMPL_API_SYSINFOAPI_HPP_
