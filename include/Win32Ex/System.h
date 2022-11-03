/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    System.hpp

Abstract:

    This Module implements the system features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_WIN32EX_SYSTEM_H_
#define _WIN32EX_WIN32EX_SYSTEM_H_

#include "Internal/version.h"
#define WIN32EX_SYSTEM_H_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SYSTEM_H_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SYSTEM_H_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "System/Ntdll.h"
#include "System/Object.h"
#include "System/Process.h"
#include "System/Service.h"

#ifdef __cplusplus
#include "System/Process.hpp"
#include "System/Service.hpp"
#include "T/sysinfoapi.hpp"

namespace Win32Ex
{
namespace System
{
#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class StringType = StringT>
#else
template <class StringType>
#endif
inline StringType SystemDirectoryT()
{
    StringType cwd(MAX_PATH, 0);
    size_t length = Win32Ex::GetSystemDirectoryT<typename StringType::value_type>(&cwd[0], (UINT)cwd.size());
    if (length > cwd.size())
    {
        cwd.resize(length);
        length = Win32Ex::GetSystemDirectoryT<typename StringType::value_type>(&cwd[0], (UINT)cwd.size());
    }
    cwd.resize(length);
    return cwd;
}
inline String SystemDirectory()
{
    return SystemDirectoryT<String>();
}
inline StringW SystemDirectorW()
{
    return SystemDirectoryT<StringW>();
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class StringType = StringT>
#else
template <class StringType>
#endif
inline StringType WindowsDirectoryT()
{
    StringType cwd(MAX_PATH, 0);
    size_t length = Win32Ex::GetWindowsDirectoryT<typename StringType::value_type>(&cwd[0], (UINT)cwd.size());
    if (length > cwd.size())
    {
        cwd.resize(length);
        length = Win32Ex::GetWindowsDirectoryT<typename StringType::value_type>(&cwd[0], (UINT)cwd.size());
    }
    cwd.resize(length);
    return cwd;
}
inline String WindowsDirectory()
{
    return WindowsDirectoryT<String>();
}
inline StringW WindowsDirectoryW()
{
    return WindowsDirectoryT<StringW>();
}

} // namespace System
} // namespace Win32Ex
#endif // __cplusplus

#endif // _WIN32EX_WIN32EX_SYSTEM_H_
