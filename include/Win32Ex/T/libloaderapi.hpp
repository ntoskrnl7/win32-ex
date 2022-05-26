/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    libloaderapi.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_TMPL_API_LIBLOADERAPI_HPP_
#define _WIN32EX_TMPL_API_LIBLOADERAPI_HPP_

#include "../Internal/version.h"
#define WIN32EX_TMPL_API_LIBLOADERAPI_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_TMPL_API_LIBLOADERAPI_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_TMPL_API_LIBLOADERAPI_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

namespace Win32Ex
{
template <typename CharType>
inline DWORD GetModuleFileNameT(_In_opt_ HMODULE hModule, _Out_ CharType *lpFilename, _In_ DWORD nSize);

template <> inline DWORD GetModuleFileNameT<CHAR>(_In_opt_ HMODULE hModule, _Out_ LPSTR lpFilename, _In_ DWORD nSize)
{
    return ::GetModuleFileNameA(hModule, lpFilename, nSize);
}
template <> inline DWORD GetModuleFileNameT<WCHAR>(_In_opt_ HMODULE hModule, _Out_ LPWSTR lpFilename, _In_ DWORD nSize)
{
    return ::GetModuleFileNameW(hModule, lpFilename, nSize);
}
} // namespace Win32Ex

#endif // _WIN32EX_TMPL_API_LIBLOADERAPI_HPP_
