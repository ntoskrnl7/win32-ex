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

#include <stdexcept>

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
    WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED_EX(Result, value_(Other.value_))

  public:
    Result Clone() const
    {
        if (isMoved_)
            throw MovedException();

        if (IsErr())
        {
            Result clone(error_);
            return clone;
        }
        Result clone(*value_);
        return clone;
    }

  private:
    void Move(Result &To)
    {
        To.isMoved_ = isMoved_;
        if (isMoved_)
            return;
        To.value_ = value_;
        To.error_ = error_;
        value_ = NULL;
        error_.ErrorCode = ERROR_SUCCESS;
        isMoved_ = true;
    }

  public:
    typedef T Type;

    Result(const Error &Error) : value_(NULL), error_(Error), isMoved_(false)
    {
    }

    Result(T Value) : value_(&Value), error_(ERROR_SUCCESS), isMoved_(false)
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
        if (error_.ErrorCode)
            throw error_;

        if (isMoved_)
            throw MovedException();

        return *value_;
    }

    T Get(T Default) const
    {
        if (error_.ErrorCode)
            return Default;

        if (isMoved_)
            throw MovedException();

        return *value_;
    }

    T operator->() const
    {
        if (error_.ErrorCode)
            throw error_;

        if (isMoved_)
            throw MovedException();

        return value_;
    }

    const Error &Error() const
    {
        return error_;
    }

    bool IsOk() const
    {
        return (!isMoved_) && (error_.ErrorCode == ERROR_SUCCESS);
    }

    bool IsErr() const
    {
        return (!isMoved_) && (error_.ErrorCode != ERROR_SUCCESS);
    }

  private:
    typename std::remove_reference<T>::type *value_;
    struct Error error_;
};

template <typename T> class Result<T, typename std::enable_if<!std::is_reference<T>::value>::type>
{
    WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED_EX(Result, value_(Other.value_))

  public:
    Result Clone() const
    {
        if (isMoved_)
            throw MovedException();

        if (IsErr())
        {
            Result clone(error_);
            return clone;
        }
        Result clone(value_);
        return clone;
    }

  private:
    void Move(Result &To)
    {
        To.isMoved_ = isMoved_;
        if (isMoved_)
            return;
        To.value_ = value_;
        To.error_ = error_;
        value_ = typename std::remove_const<T>::type();
        error_.ErrorCode = ERROR_SUCCESS;
        isMoved_ = true;
    }

  public:
    typedef T Type;

    Result(const Error &Error) : error_(Error), isMoved_(false)
    {
    }

    Result(T Value) : value_(Value), error_(ERROR_SUCCESS), isMoved_(false)
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
        if (isMoved_)
            throw MovedException();

        if (error_.ErrorCode)
            throw error_;

        return value_;
    }

    const T &Ref() const
    {
        if (isMoved_)
            throw MovedException();

        if (error_.ErrorCode)
            throw error_;

        return value_;
    }

    T Get(const T &Default) const
    {
        if (isMoved_)
            throw MovedException();

        if (error_.ErrorCode)
            return Default;

        return value_;
    }

    T operator->() const
    {
        if (isMoved_)
            throw MovedException();

        if (error_.ErrorCode)
            throw error_;

        return value_;
    }

    const Error &Error() const
    {
        return error_;
    }

    bool IsOk() const
    {
        return (!isMoved_) && (error_.ErrorCode == ERROR_SUCCESS);
    }

    bool IsErr() const
    {
        return (!isMoved_) && (error_.ErrorCode != ERROR_SUCCESS);
    }

  private:
    typename std::remove_const<T>::type value_;
    struct Error error_;
};
} // namespace Win32Ex

#endif // _WIN32EX_RESULT_HPP_