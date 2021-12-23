/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    shellapi.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_TMPL_API_SHELLAPI_HPP_
#define _WIN32EX_TMPL_API_SHELLAPI_HPP_

#include "../Internal/version.h"
#define WIN32EX_TMPL_API_SHELLAPI_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_TMPL_API_SHELLAPI_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_TMPL_API_SHELLAPI_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "macros.hpp"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")

namespace Win32Ex
{
WIN32EX_API_DEFINE_STRUCT_T(SHELLEXECUTEINFO);

template <typename _CharType>
inline BOOL ShellExecuteExT(_Inout_ typename SHELLEXECUTEINFOT<_CharType>::Type *pExecInfo);

template <> inline BOOL ShellExecuteExT<CHAR>(_Inout_ SHELLEXECUTEINFOA *pExecInfo)
{
    return ShellExecuteExA(pExecInfo);
}
template <> inline BOOL ShellExecuteExT<WCHAR>(_Inout_ SHELLEXECUTEINFOW *pExecInfo)
{
    return ShellExecuteExW(pExecInfo);
}

} // namespace Win32Ex

#endif // WIN32EX_TMPL_API_SHELLAPI_HPP
