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
#endif // __cplusplus

#endif // _WIN32EX_WIN32EX_SYSTEM_H_