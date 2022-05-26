/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    winuser.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_TMPL_API_WINUSER_HPP_
#define _WIN32EX_TMPL_API_WINUSER_HPP_

#include "../Internal/version.h"
#define WIN32EX_TMPL_API_WINUSER_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_TMPL_API_WINUSER_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_TMPL_API_WINUSER_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include <winuser.h>

namespace Win32Ex
{
template <typename CharType>
inline HDEVNOTIFY WINAPI RegisterDeviceNotificationT(_In_ HANDLE hRecipient, _In_ LPVOID NotificationFilter,
                                                     _In_ DWORD Flags);

template <>
inline HDEVNOTIFY WINAPI RegisterDeviceNotificationT<CHAR>(_In_ HANDLE hRecipient, _In_ LPVOID NotificationFilter,
                                                           _In_ DWORD Flags)
{
    return RegisterDeviceNotificationA(hRecipient, NotificationFilter, Flags);
}

template <>
inline HDEVNOTIFY WINAPI RegisterDeviceNotificationT<WCHAR>(_In_ HANDLE hRecipient, _In_ LPVOID NotificationFilter,
                                                            _In_ DWORD Flags)
{
    return RegisterDeviceNotificationW(hRecipient, NotificationFilter, Flags);
}
} // namespace Win32Ex

#endif // WIN32EX_TMPL_API_WINUSER_HPP
