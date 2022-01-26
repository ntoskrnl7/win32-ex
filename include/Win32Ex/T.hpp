/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    T.hpp

Abstract:

    This Module implements the T features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_WIN32EX_T_HPP_
#define _WIN32EX_WIN32EX_T_HPP_

#include "Internal/version.h"
#define WIN32EX_T_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_T_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_T_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "T/libloaderapi.hpp"
#include "T/processenv.hpp"
#include "T/processthreadsapi.hpp"
#include "T/shellapi.hpp"
#include "T/tlhelp32.hpp"
#include "T/winbase.hpp"
#include "T/winsvc.hpp"
#include "T/winuser.hpp"
#include "T/wtsapi32.hpp"

#endif // _WIN32EX_WIN32EX_T_HPP_