/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    processENV.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_TMPL_API_PROCESSENV_HPP_
#define _WIN32EX_TMPL_API_PROCESSENV_HPP_

#include "../Internal/version.h"
#define WIN32EX_TMPL_API_PROCESSENV_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_TMPL_API_PROCESSENV_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_TMPL_API_PROCESSENV_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

namespace Win32Ex
{
template <typename CharType> inline CharType *WINAPI GetCommandLineT(VOID);

template <> inline LPSTR WINAPI GetCommandLineT<CHAR>(VOID)
{
    return GetCommandLineA();
}

template <> inline LPWSTR WINAPI GetCommandLineT<WCHAR>(VOID)
{
    return GetCommandLineW();
}

template <typename CharType> inline DWORD GetCurrentDirectoryT(_In_ DWORD nBufferLength, _Out_ CharType *lpBuffer);

template <> inline DWORD GetCurrentDirectoryT<CHAR>(_In_ DWORD nBufferLength, _Out_ LPSTR lpBuffer)
{
    return GetCurrentDirectoryA(nBufferLength, lpBuffer);
}

template <> inline DWORD GetCurrentDirectoryT<WCHAR>(_In_ DWORD nBufferLength, _Out_ LPWSTR lpBuffer)
{
    return GetCurrentDirectoryW(nBufferLength, lpBuffer);
}
} // namespace Win32Ex

#endif // _WIN32EX_TMPL_API_PROCESSENV_HPP_
