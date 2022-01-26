/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Security.h

Abstract:

    This Module implements the security features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_WIN32EX_SECURITY_H_
#define _WIN32EX_WIN32EX_SECURITY_H_

#include "Internal/version.h"
#define WIN32EX_SECURITY_H_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SECURITY_H_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SECURITY_H_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "Security/Descriptor.h"
#include "Security/Privilege.h"
#include "Security/Sid.h"
#include "Security/Token.h"

#ifdef __cplusplus
#include "Security/Privilege.hpp"
#include "Security/Token.hpp"
#endif // __cplusplus

#endif // _WIN32EX_WIN32EX_SECURITY_H_