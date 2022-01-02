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

#if defined(__cpp_rvalue_references)
#define WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED(_CLASS)                                                                \
  private:                                                                                                             \
    bool isMoved_;                                                                                                     \
                                                                                                                       \
  public:                                                                                                              \
    bool IsMoved() const                                                                                               \
    {                                                                                                                  \
        return isMoved_;                                                                                               \
    }                                                                                                                  \
                                                                                                                       \
  public:                                                                                                              \
    _CLASS(_CLASS &&Other) : isMoved_(false)                                                                           \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &&Rhs)                                                                                    \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _CLASS(_CLASS &Other) : isMoved_(false)                                                                            \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &Rhs)                                                                                     \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _CLASS(const _CLASS &other) = delete;                                                                              \
    _CLASS &operator=(const _CLASS &rhs) = delete;
#define WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED_EX(_CLASS, _INIT_STMT)                                                 \
  private:                                                                                                             \
    bool isMoved_;                                                                                                     \
                                                                                                                       \
  public:                                                                                                              \
    bool IsMoved() const                                                                                               \
    {                                                                                                                  \
        return isMoved_;                                                                                               \
    }                                                                                                                  \
                                                                                                                       \
  public:                                                                                                              \
    _CLASS(_CLASS &&Other) : isMoved_(false), _INIT_STMT                                                               \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &&Rhs)                                                                                    \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _CLASS(_CLASS &Other) : isMoved_(false), _INIT_STMT                                                                \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &Rhs)                                                                                     \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _CLASS(const _CLASS &other) = delete;                                                                              \
    _CLASS &operator=(const _CLASS &rhs) = delete;

#define WIN32EX_MOVE_ALWAYS_CLASS(_CLASS)                                                                              \
  public:                                                                                                              \
    _CLASS(_CLASS &&Other)                                                                                             \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &&Rhs)                                                                                    \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _CLASS(_CLASS &Other)                                                                                              \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &Rhs)                                                                                     \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _CLASS(const _CLASS &other) = delete;                                                                              \
    _CLASS &operator=(const _CLASS &rhs) = delete;
#define WIN32EX_MOVE_ALWAYS_CLASS_EX(_CLASS, _INIT_STMT)                                                               \
  public:                                                                                                              \
    _CLASS(_CLASS &&Other) : _INIT_STMT                                                                                \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &&Rhs)                                                                                    \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _CLASS(_CLASS &Other) : _INIT_STMT                                                                                 \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &Rhs)                                                                                     \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _CLASS(const _CLASS &other) = delete;                                                                              \
    _CLASS &operator=(const _CLASS &rhs) = delete;
#else
#define WIN32EX_MOVE_ALWAYS_CLASS(_CLASS)                                                                              \
  public:                                                                                                              \
    _CLASS(_CLASS &Other)                                                                                              \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &Rhs)                                                                                     \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }
#define WIN32EX_MOVE_ALWAYS_CLASS_EX(_CLASS, _INIT_STMT)                                                               \
  public:                                                                                                              \
    _CLASS(_CLASS &Other) : _INIT_STMT                                                                                 \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &Rhs)                                                                                     \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }

#define WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED(_CLASS)                                                                \
  private:                                                                                                             \
    bool isMoved_;                                                                                                     \
                                                                                                                       \
  public:                                                                                                              \
    bool IsMoved() const                                                                                               \
    {                                                                                                                  \
        return isMoved_;                                                                                               \
    }                                                                                                                  \
                                                                                                                       \
  public:                                                                                                              \
    _CLASS(_CLASS &Other)                                                                                              \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &Rhs)                                                                                     \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }
#define WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED_EX(_CLASS, _INIT_STMT)                                                 \
  private:                                                                                                             \
    bool isMoved_;                                                                                                     \
                                                                                                                       \
  public:                                                                                                              \
    bool IsMoved() const                                                                                               \
    {                                                                                                                  \
        return isMoved_;                                                                                               \
    }                                                                                                                  \
                                                                                                                       \
  public:                                                                                                              \
    _CLASS(_CLASS &Other) : _INIT_STMT                                                                                 \
    {                                                                                                                  \
        Other.Move(*this);                                                                                             \
    }                                                                                                                  \
    _CLASS &operator=(_CLASS &Rhs)                                                                                     \
    {                                                                                                                  \
        Rhs.Move(*this);                                                                                               \
        return *this;                                                                                                  \
    }
#endif

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
    std::wstring ret(rhs.size() + 1, '\0');
#if defined(_MSC_VER)
    size_t len;
    if (mbstowcs_s(&len, &ret[0], ret.size(), rhs.c_str(), ret.size()))
        return L"";
#else
    size_t len = mbstowcs(&ret[0], rhs.c_str(), ret.size());
#endif
    if (len == -1)
        return L"";
    ret.resize(len);
    return ret;
}

inline std::string operator!(const std::wstring &rhs)
{
    std::string ret(rhs.size() + 1, '\0');
#if defined(_MSC_VER)
    size_t len;
    if (wcstombs_s(&len, &ret[0], ret.size(), rhs.c_str(), (int)ret.size()))
        return "";
#else
    size_t len = wcstombs(&ret[0], rhs.c_str(), (int)ret.size());
#endif
    if (len == -1)
        return "";
    ret.resize(len);
    return ret;
}
} // namespace String
} // namespace Convert

class Exception : public std::exception
{
};

class NullException : public Exception
{
};

class MovedException : public Exception
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