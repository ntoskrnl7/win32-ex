/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Result.hpp

Abstract:

    This Module implements the Result classes.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_RESULT_HPP_
#define _WIN32EX_RESULT_HPP_

#include "Internal/version.h"
#define WIN32EX_RESULT_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_RESULT_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_RESULT_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "Internal/misc.hpp"

#include <exception>

namespace Win32Ex
{
template <typename T, typename SFINAE = void> class Result;

struct Error : std::runtime_error
{
    Error(DWORD ErrorCode = GetLastError()) : ErrorCode(ErrorCode), std::runtime_error("")
    {
    }

    Error(DWORD ErrorCode, const String &Message) : ErrorCode(ErrorCode), std::runtime_error(Message.c_str())
    {
    }

    DWORD ErrorCode;
};

template <typename T> class Result<T, typename std::enable_if<std::is_reference<T>::value>::type>
{
    WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED_EX(Result, Value_(Other.Value_))

  public:
    Result Clone() const
    {
        if (IsMoved_)
            throw MovedException();

        if (IsErr())
        {
            Result clone(Error_);
            return clone;
        }
        Result clone(*Value_);
        return clone;
    }

  private:
    void Move(Result &To)
    {
        To.IsMoved_ = IsMoved_;
        if (IsMoved_)
            return;
        To.Value_ = Value_;
        To.Error_ = Error_;
        Value_ = NULL;
        Error_.ErrorCode = ERROR_SUCCESS;
        IsMoved_ = true;
    }

  public:
    typedef T Type;

    Result(const Error &Error) : Value_(NULL), Error_(Error), IsMoved_(false)
    {
    }

    Result(T Value) : Value_(&Value), Error_(ERROR_SUCCESS), IsMoved_(false)
    {
    }

#if !defined(__cpp_rvalue_references)
    Result &operator=(T Rhs)
    {
        Result(Rhs).Move(*this);
        return *this;
    }
#endif

    T Get() const
    {
        if (Error_.ErrorCode)
            throw Error_;

        if (IsMoved_)
            throw MovedException();

        return *Value_;
    }

    T Get(T Default) const
    {
        if (Error_.ErrorCode)
            return Default;

        if (IsMoved_)
            throw MovedException();

        return *Value_;
    }

    T operator->() const
    {
        if (Error_.ErrorCode)
            throw Error_;

        if (IsMoved_)
            throw MovedException();

        return Value_;
    }

    const Error &Error() const
    {
        return Error_;
    }

    bool IsOk() const
    {
        return (!IsMoved_) && (Error_.ErrorCode == ERROR_SUCCESS);
    }

    bool IsErr() const
    {
        return (!IsMoved_) && (Error_.ErrorCode != ERROR_SUCCESS);
    }

  private:
    typename std::remove_reference<T>::type *Value_;
    struct Error Error_;
};

template <typename T> class Result<T, typename std::enable_if<!std::is_reference<T>::value>::type>
{
    WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED_EX(Result, Value_(Other.Value_))

  public:
    Result Clone() const
    {
        if (IsMoved_)
            throw MovedException();

        if (IsErr())
        {
            Result clone(Error_);
            return clone;
        }
        Result clone(Value_);
        return clone;
    }

  private:
    void Move(Result &To)
    {
        To.IsMoved_ = IsMoved_;
        if (IsMoved_)
            return;
        To.Value_ = Value_;
        To.Error_ = Error_;
        Value_ = typename std::remove_const<T>::type();
        Error_.ErrorCode = ERROR_SUCCESS;
        IsMoved_ = true;
    }

  public:
    typedef T Type;

    Result(const Error &Error) : Error_(Error), IsMoved_(false)
    {
    }

    Result(T Value) : Value_(Value), Error_(ERROR_SUCCESS), IsMoved_(false)
    {
    }

#if !defined(__cpp_rvalue_references)
    Result &operator=(T Rhs)
    {
        Result(Rhs).Move(*this);
        return *this;
    }
#endif

    T Get() const
    {
        if (IsMoved_)
            throw MovedException();

        if (Error_.ErrorCode)
            throw Error_;

        return Value_;
    }

    T Get(const T &Default) const
    {
        if (IsMoved_)
            throw MovedException();

        if (Error_.ErrorCode)
            return Default;

        return Value_;
    }

    T operator->() const
    {
        if (IsMoved_)
            throw MovedException();

        if (Error_.ErrorCode)
            throw Error_;

        return Value_;
    }

    const Error &Error() const
    {
        return Error_;
    }

    bool IsOk() const
    {
        return (!IsMoved_) && (Error_.ErrorCode == ERROR_SUCCESS);
    }

    bool IsErr() const
    {
        return (!IsMoved_) && (Error_.ErrorCode != ERROR_SUCCESS);
    }

  private:
    typename std::remove_const<T>::type Value_;
    struct Error Error_;
};
} // namespace Win32Ex

#endif // _WIN32EX_RESULT_HPP_