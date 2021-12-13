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
typedef std::basic_string<WCHAR> WString;
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

template <typename T, typename SFINAE = void> class Optional;

struct None
{
};

class Exception : public std::exception
{
};

class NullException : public Exception
{
};

template <> class Optional<WString>
{
  public:
    typedef WString Type;

    Optional() : IsNone_(true)
    {
    }

    Optional(None) : IsNone_(true)
    {
    }

    Optional(PCWSTR Value) : IsNone_(false)
    {
        IsNull_ = Value == NULL;
        if (!IsNull_)
        {
            Value_ = Value;
        }
    }

    Optional(const WString &Value) : Value_(Value), IsNone_(false), IsNull_(false)
    {
    }

    operator const WString &() const
    {
        if (IsNone_)
        {
            throw Exception();
        }
        if (IsNull_)
        {
            throw NullException();
        }
        return Value_;
    }

    operator PCWSTR() const
    {
        if (IsNone_)
        {
            throw Exception();
        }
        return IsNull_ ? NULL : Value_.c_str();
    }

    bool IsSome() const
    {
        return !IsNone_;
    }

    bool IsNone() const
    {
        return IsNone_;
    }

  private:
    WString Value_;
    bool IsNull_;
    bool IsNone_;
};

template <> class Optional<String>
{
  public:
    typedef String Type;

    Optional() : IsNone_(true)
    {
    }

    Optional(None) : IsNone_(true)
    {
    }

    Optional(PCSTR Value) : IsNone_(false)
    {
        IsNull_ = Value == NULL;
        if (!IsNull_)
        {
            Value_ = Value;
        }
    }

    Optional(const String &Value) : Value_(Value), IsNone_(false), IsNull_(false)
    {
    }

    operator const String &() const
    {
        if (IsNone_)
        {
            throw Exception();
        }
        if (IsNull_)
        {
            throw NullException();
        }
        return Value_;
    }

    operator PCSTR() const
    {
        if (IsNone_)
        {
            throw Exception();
        }
        return IsNull_ ? NULL : Value_.c_str();
    }

    bool IsSome() const
    {
        return !IsNone_;
    }

    bool IsNone() const
    {
        return IsNone_;
    }

  private:
    String Value_;
    bool IsNull_;
    bool IsNone_;
};

template <typename T> class Optional<T>
{
  public:
    typedef T Type;

    Optional() : IsNone_(true)
    {
    }

    Optional(None) : IsNone_(true)
    {
    }

    Optional(T Value) : Value_(Value), IsNone_(false)
    {
    }

    operator T() const
    {
        if (IsNone_)
        {
            throw Exception();
        }
        return Value_;
    }

    bool IsSome() const
    {
        return !IsNone_;
    }

    bool IsNone() const
    {
        return IsNone_;
    }

  private:
    T Value_;
    bool IsNone_;
};

#if defined(__cpp_variadic_templates)
template <typename... Args> bool IsAll(const Optional<Args> &... args)
{
    for (auto val : {
             args.IsNone()...,
         })
    {
        if (val)
        {
            return false;
        }
    }
    return true;
}

template <typename... Args> bool IsSome(const Optional<Args> &... args)
{
    for (auto val : {
             args.IsSome()...,
         })
    {
        if (val)
        {
            return true;
        }
    }
    return false;
}

template <typename... Args> bool IsAny(const Optional<Args> &... args)
{
    return IsSome(args...);
}

template <typename... Args> bool IsNone(const Optional<Args> &... args)
{
    for (auto val : {
             args.IsSome()...,
         })
    {
        if (val)
        {
            return false;
        }
    }
    return true;
}
#endif
} // namespace Win32Ex

#ifndef __cpp_lambdas
#if defined(_MSC_VER) && _MSC_VER >= 1600
#define __cpp_lambdas 200907L
#endif
#endif

#endif // _WIN32EX_MISC_HPP_