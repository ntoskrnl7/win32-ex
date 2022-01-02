/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    wtsapi32.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_TMPL_API_WTSAPI32_HPP_
#define _WIN32EX_TMPL_API_WTSAPI32_HPP_

#include "../Internal/version.h"
#define WIN32EX_TMPL_API_WTSAPI32_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_TMPL_API_WTSAPI32_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_TMPL_API_WTSAPI32_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "macros.hpp"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

namespace Win32Ex
{
WIN32EX_API_DEFINE_STRUCT_T(WTS_SESSION_INFO);

template <typename _CharType>
inline BOOL WINAPI WTSEnumerateSessionsT(_In_ HANDLE hServer, _In_ DWORD Reserved, _In_ DWORD Version,
                                         _Out_ WTS_SESSION_INFOT<_CharType> **ppSessionInfo, _Out_ DWORD *pCount);

template <>
inline BOOL WINAPI WTSEnumerateSessionsT<WCHAR>(_In_ HANDLE hServer, _In_ DWORD Reserved, _In_ DWORD Version,
                                                _Out_ WTS_SESSION_INFOT<WCHAR> **ppSessionInfo, _Out_ DWORD *pCount)
{
    return WTSEnumerateSessionsW(hServer, Reserved, Version, (WTS_SESSION_INFOT<WCHAR>::Type **)ppSessionInfo, pCount);
}

template <>
inline BOOL WINAPI WTSEnumerateSessionsT<CHAR>(_In_ HANDLE hServer, _In_ DWORD Reserved, _In_ DWORD Version,
                                               _Out_ WTS_SESSION_INFOT<CHAR> **ppSessionInfo, _Out_ DWORD *pCount)
{
    return WTSEnumerateSessionsA(hServer, Reserved, Version, (WTS_SESSION_INFOT<CHAR>::Type **)ppSessionInfo, pCount);
}

template <typename _CharType>
inline BOOL WINAPI WTSEnumerateSessionsT(_In_ HANDLE hServer, _In_ DWORD Reserved, _In_ DWORD Version,
                                         _Out_ typename WTS_SESSION_INFOT<_CharType>::Type **ppSessionInfo,
                                         _Out_ DWORD *pCount);

template <>
inline BOOL WINAPI WTSEnumerateSessionsT<WCHAR>(_In_ HANDLE hServer, _In_ DWORD Reserved, _In_ DWORD Version,
                                                _Out_ typename WTS_SESSION_INFOT<WCHAR>::Type **ppSessionInfo,
                                                _Out_ DWORD *pCount)
{
    return WTSEnumerateSessionsW(hServer, Reserved, Version, ppSessionInfo, pCount);
}

template <>
inline BOOL WINAPI WTSEnumerateSessionsT<CHAR>(_In_ HANDLE hServer, _In_ DWORD Reserved, _In_ DWORD Version,
                                               _Out_ typename WTS_SESSION_INFOT<CHAR>::Type **ppSessionInfo,
                                               _Out_ DWORD *pCount)
{
    return WTSEnumerateSessionsA(hServer, Reserved, Version, ppSessionInfo, pCount);
}

template <typename _CharType>
inline BOOL WINAPI WTSQuerySessionInformationT(_In_ HANDLE hServer, _In_ DWORD SessionId,
                                               _In_ WTS_INFO_CLASS WTSInfoClass, _Out_ _CharType **ppBuffer,
                                               _Out_ DWORD *pBytesReturned);

template <>
inline BOOL WINAPI WTSQuerySessionInformationT<CHAR>(_In_ HANDLE hServer, _In_ DWORD SessionId,
                                                     _In_ WTS_INFO_CLASS WTSInfoClass, _Out_ LPSTR *ppBuffer,
                                                     _Out_ DWORD *pBytesReturned)
{
    return WTSQuerySessionInformationA(hServer, SessionId, WTSInfoClass, ppBuffer, pBytesReturned);
}

template <>
inline BOOL WINAPI WTSQuerySessionInformationT<WCHAR>(_In_ HANDLE hServer, _In_ DWORD SessionId,
                                                      _In_ WTS_INFO_CLASS WTSInfoClass, _Out_ LPWSTR *ppBuffer,
                                                      _Out_ DWORD *pBytesReturned)
{
    return WTSQuerySessionInformationW(hServer, SessionId, WTSInfoClass, ppBuffer, pBytesReturned);
}
} // namespace Win32Ex

#endif // WIN32EX_TMPL_API_WTSAPI32_HPP
