/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    StlCompat.hpp

Abstract:

    This Module implements the C++ STL helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef __cpp_lambdas
#if defined(_MSC_VER) && _MSC_VER >= 1600
#define __cpp_lambdas 200907L
#endif
#endif

#if defined(_MSC_VER) && _MSC_VER < 1600
#define CXX_USE_STD_ENABLE_IF
#define CXX_USE_STD_INTEGRAL_CONSTANT
#define CXX_USE_STD_REMOVE_REF
#define CXX_USE_STD_REMOVE_CONST
#define CXX_USE_STD_IS_REF
#include <ext/stl_compat>
#endif