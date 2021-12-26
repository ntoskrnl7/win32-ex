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
namespace std
{
template <class T> struct remove_const
{
    typedef T type;
};
template <class T> struct remove_const<const T>
{
    typedef T type;
};
template <class T> struct remove_reference
{
    typedef T type;
};
template <class T> struct remove_reference<T &>
{
    typedef T type;
};
} // namespace std
#endif