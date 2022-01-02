/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    tlhelp32.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_TMPL_API_TLHELP32_HPP_
#define _WIN32EX_TMPL_API_TLHELP32_HPP_

#include "../Internal/version.h"
#define WIN32EX_TMPL_API_TLHELP32_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_TMPL_API_TLHELP32_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_TMPL_API_TLHELP32_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "macros.hpp"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <tlhelp32.h>

#if defined(UNICODE)
#undef Process32Next
#undef Process32First
#undef PROCESSENTRY32
#undef PPROCESSENTRY32
#undef LPPROCESSENTRY32
#endif

namespace Win32Ex
{
WIN32EX_API_DEFINE_STRUCT_T_EX(PROCESSENTRY32, PROCESSENTRY32T, PROCESSENTRY32, PROCESSENTRY32W);

template <typename _CharType>
inline BOOL Process32FirstT(HANDLE hSnapshot, typename PROCESSENTRY32T<_CharType>::Type *lppe);

template <> inline BOOL Process32FirstT<CHAR>(HANDLE hSnapshot, LPPROCESSENTRY32 lppe)
{
    return ::Process32First(hSnapshot, lppe);
}

template <> inline BOOL Process32FirstT<WCHAR>(HANDLE hSnapshot, LPPROCESSENTRY32W lppe)
{
    return ::Process32FirstW(hSnapshot, lppe);
}

template <typename _CharType>
inline BOOL Process32NextT(HANDLE hSnapshot, typename PROCESSENTRY32T<_CharType>::Type *lppe);

template <> inline BOOL Process32NextT<CHAR>(HANDLE hSnapshot, LPPROCESSENTRY32 lppe)
{
    return ::Process32Next(hSnapshot, lppe);
}

template <> inline BOOL Process32NextT<WCHAR>(HANDLE hSnapshot, LPPROCESSENTRY32W lppe)
{
    return ::Process32NextW(hSnapshot, lppe);
}
} // namespace Win32Ex

#if defined(UNICODE)
#define Process32Next Process32NextW
#define Process32First Process32FirstW
#define PROCESSENTRY32 PROCESSENTRY32W
#define PPROCESSENTRY32 PPROCESSENTRY32W
#define LPPROCESSENTRY32 LPPROCESSENTRY32W
#endif

#endif // _WIN32EX_TMPL_API_TLHELP32_HPP_
