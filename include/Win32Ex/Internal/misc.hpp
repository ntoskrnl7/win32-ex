/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    misc.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_MISC_HPP_
#define _WIN32EX_MISC_HPP_

#include "version.h"
#define WIN32EX_MISC_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_MISC_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_MISC_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "misc.h"
#include <string>
#include <tchar.h>

#if defined(_MSC_VER)
#include <atlconv.h>
#endif

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <climits>
#include <windows.h>

#define WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T
#if defined(_MSC_VER) && _MSC_VER < 1800
#undef WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T
#endif

namespace Win32Ex
{
class Duration
{
  public:
    static Duration Infinite()
    {
        return Duration(INFINITE);
    }

    static Duration Second(unsigned long Second)
    {
        return Duration(((ULONG_MAX / 1000) < Second) ? ULONG_MAX : Second * 1000);
    }

  public:
    Duration(unsigned long Millisecond) : Millisecond_(Millisecond)
    {
    }

    operator DWORD() const
    {
        return Millisecond_;
    }

    Duration &operator+=(Duration rhs)
    {
        Millisecond_ += rhs.Millisecond_;
        return *this;
    }

    Duration &operator-=(Duration rhs)
    {
        Millisecond_ -= rhs.Millisecond_;
        return *this;
    }

    Duration &operator*=(Duration rhs)
    {
        Millisecond_ *= rhs.Millisecond_;
        return *this;
    }

    Duration &operator/=(Duration rhs)
    {
        Millisecond_ *= rhs.Millisecond_;
        return *this;
    }

  private:
    DWORD Millisecond_;
};

class WaitableObject
{
    friend class Waitable;

  private:
    virtual bool IsWaitable() = 0;
    virtual bool Wait(Duration Timeout) = 0;
};

class Waitable
{
    friend class WaitableObject;

  public:
    Waitable(WaitableObject &object) : object_(object)
    {
    }

  public:
    operator bool()
    {
        return object_.IsWaitable();
    }

    bool Wait(Duration Timeout = Duration::Infinite())
    {
        if (!object_.IsWaitable())
            return false;

        return object_.Wait(Timeout);
    }

  private:
    WaitableObject &object_;
};

typedef std::basic_string<CHAR> String;
typedef std::basic_string<WCHAR> StringW;
typedef std::basic_string<TCHAR> StringT;

namespace Convert
{
namespace String
{
inline std::wstring operator!(const std::string &rhs)
{
#if defined(_MSC_VER)
    USES_CONVERSION;
    return A2W(rhs.c_str());
#else
    std::wstring ret(rhs.size() + 1, '\0');
    size_t len = mbstowcs(&ret[0], rhs.c_str(), ret.size());
    if (len == -1)
        return L"";
    ret.resize(len);
    return ret;
#endif
}

inline std::string operator!(const std::wstring &rhs)
{
#if defined(_MSC_VER)
    USES_CONVERSION;
    return W2A(rhs.c_str());
#else
    std::string ret(rhs.size() + 1, '\0');
    size_t len = wcstombs(&ret[0], rhs.c_str(), (int)ret.size());
    if (len == -1)
        return "";
    ret.resize(len);
    return ret;
#endif
}
} // namespace String
} // namespace Convert

class Exception : public std::exception
{
};

class NullException : public Exception
{
};

namespace Details
{
template <class _StringType> struct ConstValue
{
};

template <> struct ConstValue<String>
{
    static const String &Empty()
    {
        static String empty;
        return empty;
    }
};

template <> struct ConstValue<StringW>
{
    static const StringW &Empty()
    {
        static StringW empty;
        return empty;
    }
};
} // namespace Details
} // namespace Win32Ex

#include "StlCompat.hpp"

#endif // _WIN32EX_MISC_HPP_