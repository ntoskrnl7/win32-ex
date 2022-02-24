/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Termserv.hpp

Abstract:

    This Module implements the Session class.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SYSTEM_SESSION_HPP_
#define _WIN32EX_SYSTEM_SESSION_HPP_

#include "../Internal/version.h"
#define WIN32EX_SYSTEM_SESSION_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SYSTEM_SESSION_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SYSTEM_SESSION_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

#include <list>
#include <memory>

#include "../Internal/misc.hpp"
#include "../Result.hpp"
#include "../T/wtsapi32.hpp"
#include "Process.hpp"

namespace Win32Ex
{
namespace System
{
template <class _StringType = StringT> class SessionT
{
    WIN32EX_MOVE_ONLY_CLASS_WITH_IS_MOVED(SessionT)

  public:
    SessionT Clone() const
    {
        if (isMoved_)
            throw MovedException();

        SessionT clone(id_);
        return clone;
    }

  private:
    void Move(SessionT &To)
    {
        To.isMoved_ = isMoved_;
        if (isMoved_)
            return;

        To.id_ = id_;
        To.name_.swap(name_);
        To.userName_.swap(userName_);
        id_ = MAXDWORD;
        isMoved_ = true;
    }

  public:
    typedef _StringType StringType;
    typedef typename StringType::value_type CharType;
    typedef RunnableProcessT<StringType> RunnableProcess;
    typedef SharedPtr<RunnableProcess> RunnableProcessPtr;

  public:
    static std::list<SharedPtr<SessionT>> All()
    {
        std::list<SharedPtr<SessionT>> all;
        WTS_SESSION_INFOT<CharType> *sessionInfo = NULL;
        DWORD count = 0;
        if (WTSEnumerateSessionsT<CharType>(WTS_CURRENT_SERVER, 0, 1, &sessionInfo, &count))
        {
            for (DWORD i = 0; i < count; ++i)
#if (defined(_MSC_VER) && _MSC_VER < 1600) || defined(_VARIADIC_EXPAND_0X)
                all.push_back(SharedPtr<SessionT>(new SessionT(sessionInfo[i])));
#else
                all.push_back(std::make_shared<SessionT>(sessionInfo[i]));
#endif
            WTSFreeMemory(sessionInfo);
        }
        return all;
    }

  private:
    void Init_()
    {
        DWORD bytesReturned = 0;
        CharType *value;
        if (name_.empty())
        {
            if (WTSQuerySessionInformationT<CharType>(WTS_CURRENT_SERVER_HANDLE, id_, WTSWinStationName, &value,
                                                      &bytesReturned))
            {
                name_ = value;
                WTSFreeMemory(value);
            }
        }

        if (userName_.empty())
        {
            if (WTSQuerySessionInformationT<CharType>(WTS_CURRENT_SERVER_HANDLE, id_, WTSUserName, &value,
                                                      &bytesReturned))
            {
                userName_ = value;
                WTSFreeMemory(value);
            }
        }
    }

  public:
    SessionT(DWORD SessionId) : id_(SessionId), isMoved_(false)
    {
        Init_();
    }

    SessionT(const WTS_SESSION_INFOT<CharType> &Info)
        : id_(Info.SessionId), name_(Info.pWinStationName), isMoved_(false)
    {
        Init_();
    }

    Result<RunnableProcessPtr> NewProcess(Win32Ex::System::ProcessAccountType Type, const StringType &Command,
                                          const Optional<const StringType &> &Arguments = None(),
                                          const Optional<const StringType &> &CurrentDirectory = None(),
                                          DWORD CreationFlags = 0L,
                                          const typename Win32Ex::STARTUPINFOT<CharType>::Type *StartupInfo = NULL,
                                          BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL) const
    {
        try
        {
            if (Type == UserAccount)
            {
#if (defined(_MSC_VER) && _MSC_VER < 1600) || defined(_VARIADIC_EXPAND_0X)
                return RunnableProcessPtr(new RunnableSessionProcessT<UserAccount, StringType>(
                    id_, Command, Arguments, CurrentDirectory, CreationFlags, StartupInfo, InheritHandles,
                    EnvironmentBlock));
#else
                return (RunnableProcessPtr)std::make_shared<RunnableSessionProcessT<UserAccount, StringType>>(
                    id_, Command, Arguments, CurrentDirectory, CreationFlags, StartupInfo, InheritHandles,
                    EnvironmentBlock);
#endif
            }
            else if (Type == SystemAccount)
            {
#if (defined(_MSC_VER) && _MSC_VER < 1600) || defined(_VARIADIC_EXPAND_0X)
                return RunnableProcessPtr(new RunnableSessionProcessT<SystemAccount, StringType>(
                    id_, Command, Arguments, CurrentDirectory, CreationFlags, StartupInfo, InheritHandles,
                    EnvironmentBlock));
#else
                return (RunnableProcessPtr)std::make_shared<RunnableSessionProcessT<SystemAccount, StringType>>(
                    id_, Command, Arguments, CurrentDirectory, CreationFlags, StartupInfo, InheritHandles,
                    EnvironmentBlock);
#endif
            }
            return Error(ERROR_UNSUPPORTED_TYPE, "Invalid process account type");
        }
        catch (const std::exception &e)
        {
            return Error(ERROR_FUNCTION_FAILED, e.what());
        }
    }

    DWORD Id() const
    {
        return id_;
    }

    WTS_CONNECTSTATE_CLASS State() const
    {
        WTS_CONNECTSTATE_CLASS result;
        DWORD bytesReturned = 0;
        CharType *value;

        if (WTSQuerySessionInformationT<CharType>(WTS_CURRENT_SERVER_HANDLE, id_, WTSConnectState, &value,
                                                  &bytesReturned))
        {
            result = *(WTS_CONNECTSTATE_CLASS *)value;
            WTSFreeMemory(value);
            return result;
        }
        throw FailureException();
    }

    const StringType &Name() const
    {
        return name_;
    }

    const StringType &UserName() const
    {
        return userName_;
    }

  private:
    DWORD id_;
    StringType name_;
    StringType userName_;
};
typedef SessionT<String> Session;
typedef SessionT<StringW> SessionW;
} // namespace System

namespace ThisSession
{
DWORD Id()
{
    return WTSGetActiveConsoleSessionId();
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class _StringType = StringT>
#else
template <class _StringType>
#endif
_StringType NameT()
{
    return System::SessionT<_StringType>(ThisSession::Id()).Name();
}

String Name()
{
    return NameT<String>();
}

StringW NameW()
{
    return NameT<StringW>();
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class _StringType = StringT>
#else
template <class _StringType>
#endif
_StringType UserNameT()
{
    return System::SessionT<_StringType>(ThisSession::Id()).UserName();
}

String UserName()
{
    return UserNameT<String>();
}

StringW UserNameW()
{
    return UserNameT<StringW>();
}

WTS_CONNECTSTATE_CLASS State()
{
    return System::Session(ThisSession::Id()).State();
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class _StringType = StringT>
#else
template <class _StringType>
#endif
Result<typename System::SessionT<_StringType>::RunnableProcessPtr> NewProcessT(
    System::ProcessAccountType Type, const std::basic_string<typename _StringType::value_type> &Command,
    const Optional<const _StringType &> &Arguments = None(),
    const Optional<const _StringType &> &CurrentDirectory = None(), DWORD CreationFlags = 0L,
    const typename STARTUPINFOT<typename _StringType::value_type>::Type *StartupInfo = NULL,
    BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL)
{
    return System::SessionT<_StringType>(ThisSession::Id())
        .NewProcess(Type, Command, Arguments, CurrentDirectory, CreationFlags, StartupInfo, InheritHandles,
                    EnvironmentBlock);
}

Result<typename System::SessionT<String>::RunnableProcessPtr> NewProcess(
    System::ProcessAccountType Type, const std::basic_string<typename String::value_type> &Command,
    const Optional<const String &> &Arguments = None(), const Optional<const String &> &CurrentDirectory = None(),
    DWORD CreationFlags = 0L, const STARTUPINFOT<CHAR>::Type *StartupInfo = NULL, BOOL InheritHandles = FALSE,
    LPVOID EnvironmentBlock = NULL)
{
    return NewProcessT<String>(Type, Command, Arguments, CurrentDirectory, CreationFlags, StartupInfo, InheritHandles,
                               EnvironmentBlock);
}

Result<typename System::SessionT<StringW>::RunnableProcessPtr> NewProcessW(
    System::ProcessAccountType Type, const std::basic_string<typename StringW::value_type> &Command,
    const Optional<const StringW &> &Arguments = None(), const Optional<const StringW &> &CurrentDirectory = None(),
    DWORD CreationFlags = 0L, const STARTUPINFOT<WCHAR>::Type *StartupInfo = NULL, BOOL InheritHandles = FALSE,
    LPVOID EnvironmentBlock = NULL)
{
    return NewProcessT<StringW>(Type, Command, Arguments, CurrentDirectory, CreationFlags, StartupInfo, InheritHandles,
                                EnvironmentBlock);
}
} // namespace ThisSession
} // namespace Win32Ex

#endif //_WIN32EX_SYSTEM_SESSION_HPP_
