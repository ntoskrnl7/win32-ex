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
template <bool B, class T = void> struct enable_if
{
};

template <class T> struct enable_if<true, T>
{
    typedef T type;
};

template <class T, T v> struct integral_constant
{
    static const T value = v;
    typedef T value_type;
    typedef integral_constant type;
    operator value_type() const
    {
        return value;
    }
};
typedef std::integral_constant<bool, true> true_type;
typedef std::integral_constant<bool, false> false_type;

template <class T> struct is_reference : std::false_type
{
};
template <class T> struct is_reference<T &> : std::true_type
{
};

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