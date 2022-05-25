/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    winbase.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_TMPL_API_WINBASE_HPP_
#define _WIN32EX_TMPL_API_WINBASE_HPP_

#include "../Internal/version.h"
#define WIN32EX_TMPL_API_WINBASE_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_TMPL_API_WINBASE_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_TMPL_API_WINBASE_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include <WinBase.h>

namespace Win32Ex
{
template <typename CharType>
inline DWORD WINAPI FormatMessageT(_In_ DWORD dwFlags, _In_opt_ LPCVOID lpSource, _In_ DWORD dwMessageId,
                                   _In_ DWORD dwLanguageId, _Out_ CharType *lpBuffer, _In_ DWORD nSize,
                                   _In_opt_ va_list *Arguments);

template <>
inline DWORD WINAPI FormatMessageT<CHAR>(_In_ DWORD dwFlags, _In_opt_ LPCVOID lpSource, _In_ DWORD dwMessageId,
                                         _In_ DWORD dwLanguageId, _Out_ LPSTR lpBuffer, _In_ DWORD nSize,
                                         _In_opt_ va_list *Arguments)
{
    return FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);
}

template <>
inline DWORD WINAPI FormatMessageT<WCHAR>(_In_ DWORD dwFlags, _In_opt_ LPCVOID lpSource, _In_ DWORD dwMessageId,
                                          _In_ DWORD dwLanguageId, _Out_ LPWSTR lpBuffer, _In_ DWORD nSize,
                                          _In_opt_ va_list *Arguments)
{
    return FormatMessageW(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);
}

template <typename CharType>
inline BOOL WINAPI LookupPrivilegeNameT(_In_opt_ CONST CharType *lpSystemName, _In_ PLUID lpLuid,
                                        _Out_ CharType *lpName, _Inout_ LPDWORD cchName);

template <>
inline BOOL WINAPI LookupPrivilegeNameT<CHAR>(_In_opt_ LPCSTR lpSystemName, _In_ PLUID lpLuid, _Out_ LPSTR lpName,
                                              _Inout_ LPDWORD cchName)
{
    return LookupPrivilegeNameA(lpSystemName, lpLuid, lpName, cchName);
}

template <>
inline BOOL WINAPI LookupPrivilegeNameT<WCHAR>(_In_opt_ LPCWSTR lpSystemName, _In_ PLUID lpLuid, _Out_ LPWSTR lpName,
                                               _Inout_ LPDWORD cchName)
{
    return LookupPrivilegeNameW(lpSystemName, lpLuid, lpName, cchName);
}

template <typename CharType>
inline BOOL WINAPI LookupPrivilegeValueT(_In_opt_ CONST CharType *lpSystemName, _In_ CONST CharType *lpName,
                                         _Out_ PLUID lpLuid);

template <>
inline BOOL WINAPI LookupPrivilegeValueT<CHAR>(_In_opt_ LPCSTR lpSystemName, _In_ LPCSTR lpName, _Out_ PLUID lpLuid)
{
    return LookupPrivilegeValueA(lpSystemName, lpName, lpLuid);
}

template <>
inline BOOL WINAPI LookupPrivilegeValueT<WCHAR>(_In_opt_ LPCWSTR lpSystemName, _In_ LPCWSTR lpName, _Out_ PLUID lpLuid)
{
    return LookupPrivilegeValueW(lpSystemName, lpName, lpLuid);
}
} // namespace Win32Ex

#endif // WIN32EX_TMPL_API_WINBASE_HPP
