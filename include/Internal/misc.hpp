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
#include <Windows.h>
#include <climits>

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

typedef std::basic_string<TCHAR> TString;
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
} // namespace Win32Ex

#ifndef __cpp_lambdas
#if defined(_MSC_VER) && _MSC_VER >= 1600
#define __cpp_lambdas 200907L
#endif
#endif