/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Sid.hpp

Abstract:

    This Module implements the Sid class.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SECURITY_SID_HPP_
#define _WIN32EX_SECURITY_SID_HPP_

#include "../Internal/version.h"
#define WIN32EX_SECURITY_SID_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SECURITY_SID_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SECURITY_SID_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "../Internal/misc.hpp"
#include "Privilege.hpp"
#include "Token.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <functional>

namespace Win32Ex
{
namespace Security
{
class Sid {
static const Sid& WellknownSid() {

}
};
}
}

#endif // _WIN32EX_SECURITY_SID_HPP_