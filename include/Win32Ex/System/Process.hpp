/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Process.hpp

Abstract:

    This Module implements the SystemAccountProcess/UserAccountProcess class.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SYSTEM_PROCESS_HPP_
#define _WIN32EX_SYSTEM_PROCESS_HPP_

#include "../Internal/version.h"
#define WIN32EX_SYSTEM_PROCESS_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SYSTEM_PROCESS_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SYSTEM_PROCESS_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <Windows.h>

#include "../TmplApi/shellapi.hpp"

#include "../Internal/misc.hpp"
#include "../Security/Token.h"
#include "Process.h"
#include <algorithm>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>

#if defined(_MSC_VER) && _MSC_VER < 1600
#define _STD_NS_ std::tr1
#else
#define _STD_NS_ std
#endif

namespace Win32Ex
{

namespace ThisProcess
{
static std::string &GetExecutablePath()
{
    static std::string executablePath_(MAX_PATH, '\0');
    size_t returnSize = GetModuleFileNameA(NULL, &executablePath_[0], (DWORD)executablePath_.size());
    for (;;)
    {
        if (returnSize < executablePath_.size())
        {
            executablePath_.resize(returnSize);
            break;
        }
        else
        {
            executablePath_.resize(returnSize + MAX_PATH);
            returnSize = GetModuleFileNameA(NULL, &executablePath_[0], (DWORD)executablePath_.size());
            executablePath_.resize(returnSize);
        }
        if (GetLastError() == ERROR_SUCCESS)
        {
            break;
        }
    }
    return executablePath_;
}

namespace Details
{
static DWORD mainThreadId = GetCurrentThreadId();
static HANDLE OpenMainThread();
static HANDLE mainThreadHandle = OpenMainThread();

static VOID CloseMainThread()
{
    if (mainThreadHandle != NULL)
        CloseHandle(mainThreadHandle);
}

static HANDLE OpenMainThread()
{
    HANDLE handle = OpenThread(MAXIMUM_ALLOWED, FALSE, GetCurrentThreadId());
    if (handle != NULL)
        atexit(CloseMainThread);
    return handle;
}
} // namespace Details

static DWORD GetMainThreadId()
{
    return Details::mainThreadId;
}

static HANDLE GetMainThreadHandle()
{
    return Details::mainThreadHandle;
}

static DWORD GetId()
{
    return GetCurrentProcessId();
}

static HANDLE GetHandle()
{
    return GetCurrentProcess();
}

static String GetCurrentDirectory()
{
    String cwd(MAX_PATH, '\0');
    size_t length = ::GetCurrentDirectoryA((DWORD)cwd.size(), &cwd[0]);
    if (length > cwd.size())
    {
        cwd.resize(length);
        length = ::GetCurrentDirectoryA((DWORD)cwd.size(), &cwd[0]);
    }
    cwd.resize(length);
    return cwd;
}

static bool IsAdmin()
{
    return IsUserAdmin(GetCurrentProcessToken()) == TRUE;
}
} // namespace ThisProcess
namespace System
{
typedef DWORD ProcessId;

struct ProcessHandle
{
    HANDLE value;

    static ProcessHandle FromHANDLE(HANDLE Handle)
    {
        ProcessHandle handle;
        handle.value = Handle;
        return handle;
    }
};

//
//
//

template <typename _CharType> struct ConstValue
{
};

template <> struct ConstValue<CHAR>
{
    static PCSTR Space()
    {
        return " ";
    }
    static PCSTR RunAs()
    {
        return "runas";
    }
};
template <> struct ConstValue<WCHAR>
{
    static PCWSTR Space()
    {
        return L" ";
    }
    static PCWSTR RunAs()
    {
        return L"runas";
    }
};

//
//
//

template <typename _StringType> class BasicRunnableProcess;

template <typename _StringType> class BasicProcess : public WaitableObject
{
  protected:
    friend class BasicRunnableProcess<_StringType>;
    typedef typename _StringType::value_type _CharType;

    void Attach_(HANDLE ProcessHandle)
    {
        DWORD processId = GetProcessId(ProcessHandle);
        if (processId != 0)
            ProcessIdToSessionId(processId, &sessionId_);

        DWORD length = 10;
        _StringType executablePath;
        executablePath.resize(length);
        while (!QueryFullProcessImageNameT<_CharType>(ProcessHandle, 0, &executablePath[0], &length))
        {
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            {
                length = 0;
                break;
            }
            length += 32;
            executablePath.resize(length);
        }

        if (length)
        {
            executablePath.resize(length);
            executablePath_.swap(executablePath);
        }

        processInfo_.hProcess = ProcessHandle;
        processInfo_.dwProcessId = processId;
        processInfo_.dwThreadId = 0;
        processInfo_.hThread = NULL;
    }

    void Detach_()
    {
        HANDLE handle;
        handle = InterlockedExchangePointer(&processInfo_.hProcess, NULL);
        if (handle)
            CloseHandle(handle);

        handle = InterlockedExchangePointer(&processInfo_.hThread, NULL);
        if (handle)
            CloseHandle(handle);
    }

    void Init_()
    {
        ZeroMemory(&processInfo_, sizeof(processInfo_));
        sessionId_ = WTSGetActiveConsoleSessionId();
    }

    BasicProcess()
    {
        Init_();
    }

  public:
#if defined(__cpp_rvalue_references)
    BasicProcess(BasicProcess &&other)
    {
        Init_();
        Swap(other);
    }

    BasicProcess &operator=(BasicProcess &&rhs)
    {
        return Swap(rhs);
    }

    BasicProcess(const BasicProcess &other) = delete;
    BasicProcess &operator=(const BasicProcess &rhs) = delete;
#else
    BasicProcess(BasicProcess &other)
    {
        Init_();
        Swap(other);
    }

    BasicProcess &operator=(BasicProcess &rhs)
    {
        return Swap(rhs);
    }
#endif

  public:
    BasicProcess(ProcessHandle ProcessHandle)
    {
        Init_();
        Attach_(ProcessHandle.value);
    }

    BasicProcess(ProcessId ProcessId)
    {
        Init_();
        HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessId);
        if (hProcess)
            Attach_(hProcess);
    }

    ~BasicProcess()
    {
        Detach_();
    }

    BOOL Attach(ProcessHandle ProcessHandle)
    {
        if (processInfo_.hProcess)
        {
            SetLastError(ERROR_ALREADY_INITIALIZED);
            return FALSE;
        }
        Attach_(ProcessHandle.value);
        return TRUE;
    }

    BasicProcess &Swap(BasicProcess &other)
    {
        std::swap(processInfo_, other.processInfo_);
        std::swap(sessionId_, other.sessionId_);
        executablePath_.swap(other.executablePath_);
        return *this;
    }

    bool IsValid() const
    {
        return processInfo_.dwProcessId != 0;
    }

    bool IsRunning() const
    {
        if (processInfo_.hProcess == NULL)
            return false;
        return WaitForSingleObject(processInfo_.hProcess, 0) == WAIT_TIMEOUT;
    }

    const _StringType &GetExecutablePath() const
    {
        return executablePath_;
    }

    operator const _StringType() const
    {
        std::basic_stringstream<typename _StringType::value_type> process_info;
        if (!processInfo_.dwProcessId)
        {
            return "Invalid process";
        }
        process_info << executablePath_ << "(" << processInfo_.dwProcessId << ")";
        return process_info.str();
    }

    DWORD GetId() const
    {
        return processInfo_.dwProcessId;
    }

    HANDLE GetHandle() const
    {
        return processInfo_.hProcess;
    }

    BasicProcess<_StringType> GetParent()
    {
        return BasicProcess<_StringType>(GetParentProcessId(processInfo_.dwProcessId));
    }

  protected:
    DWORD sessionId_;
    _StringType executablePath_;
    PROCESS_INFORMATION processInfo_;

    //
    // WaitableObject Interface Implementations.
    //
  public:
    bool IsWaitable()
    {
        return processInfo_.hProcess != NULL;
    }

    bool Wait(Duration Timeout)
    {
        return WaitForSingleObject(processInfo_.hProcess, Timeout) == WAIT_OBJECT_0;
    }
};

enum ProcessAccountType
{
    SystemAccount,
    UserAccount
};

template <typename _StringType, ProcessAccountType _Type> class BasicRunnableSessionProcess;
template <typename _StringType> class BasicElevatedProcess;

template <typename _StringType> class BasicRunnableProcess : public BasicProcess<_StringType>
{
  protected:
    friend class BasicRunnableSessionProcess<_StringType, SystemAccount>;
    friend class BasicRunnableSessionProcess<_StringType, UserAccount>;
    friend class BasicElevatedProcess<_StringType>;
    friend class BasicElevatedProcess<_StringType>;

    typedef BasicProcess<_StringType> _BasicProcess;
    typedef typename _StringType::value_type _CharType;

  private:
    typedef std::function<void(BasicRunnableProcess &)> ProcessEnterCallbackWithSelf;
    typedef std::function<void(BasicRunnableProcess &)> ProcessExitCallbackWithSelf;
    typedef std::function<void(BasicRunnableProcess &, DWORD, const std::exception &)> ProcessErrorCallbackWithSelf;

    typedef std::function<void()> ProcessEnterCallback;
    typedef std::function<void()> ProcessExitCallback;
    typedef std::function<void(DWORD, const std::exception &)> ProcessErrorCallback;

    void InitCallbacks_()
    {
        enterCallbackWithSelf_ =
            _STD_NS_::bind(&BasicRunnableProcess::EnterCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1);
        exitCallbackWithSelf_ =
            _STD_NS_::bind(&BasicRunnableProcess::ExitCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1);
        errorCallbackWithSelf_ =
            _STD_NS_::bind(&BasicRunnableProcess::ErrorCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1,
                           _STD_NS_::placeholders::_2, _STD_NS_::placeholders::_3);
    }

    void EnterCallbackWithSelfDefault_(BasicRunnableProcess &)
    {
        if (enterCallback_)
            enterCallback_();
    };

    void ExitCallbackWithSelfDefault_(BasicRunnableProcess &)
    {
        if (exitCallback_)
            exitCallback_();
    };

    void ErrorCallbackWithSelfDefault_(BasicRunnableProcess &, DWORD LastError, const std::exception &e)
    {
        if (errorCallback_)
            errorCallback_(LastError, e);
    };

    static DWORD WINAPI ExitDetectionThreadProc_(PVOID lpThreadParameter)
    {
        BasicRunnableProcess *self = (BasicRunnableProcess *)lpThreadParameter;
        HANDLE handles[2] = {self->hThreadStopEvent_, self->processInfo_.hProcess};
        switch (WaitForMultipleObjects(2, handles, FALSE, INFINITE))
        {
        case WAIT_OBJECT_0:
        case WAIT_ABANDONED_0:
            // 스레드 종료 이벤트가 시그널되었다면, 스레드를 종료합니다.
            return 0;

        case WAIT_OBJECT_0 + 1:
        case WAIT_ABANDONED_0 + 1:
            // 프로세스가 종료되었다면, 핸들을 정리합니다.
            CloseHandle(InterlockedExchangePointer(&self->processInfo_.hProcess, NULL));
            CloseHandle(InterlockedExchangePointer(&self->processInfo_.hThread, NULL));
            break;

        case WAIT_FAILED:
        default:
            // 실패 시, 에러 콜백을 호출합니다.
            if (self->errorCallback_)
                self->errorCallbackWithSelf_(*self, GetLastError(),
                                             std::runtime_error("Failed to WaitForSingleObject"));
            break;
        }

        // 프로세스 종료 콜백을 호출합니다 (작업 실패 시에도 호출하도록 처리했습니다.)
        if (self->exitCallback_)
            self->exitCallbackWithSelf_(*self);

        CloseHandle(InterlockedExchangePointer(&self->exitDetectionThread_, NULL));
        return 0;
    }

  protected:
    BasicRunnableProcess(const _StringType &ExecutablePath, const _StringType &Arguments = _StringType(),
                         const _StringType &CurrentDirectory = _StringType(), DWORD CreationFlags = 0L,
                         const typename Win32Ex::STARTUPINFOT<_CharType>::Type *StartupInfo = NULL,
                         BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL)
        : _BasicProcess(), exitDetectionThread_(NULL), hThreadStopEvent_(NULL), arguments_(Arguments),
          currentDirectory_(CurrentDirectory), creationFlags_(CreationFlags), startupInfo_(StartupInfo),
          inheritHandles_(InheritHandles), environmentBlock_(EnvironmentBlock)
    {
        _BasicProcess::executablePath_ = ExecutablePath;
        InitCallbacks_();
    }

  public:
    ~BasicRunnableProcess()
    {
        HANDLE handle = InterlockedExchangePointer(&hThreadStopEvent_, NULL);
        if (handle)
        {
            SetEvent(handle);
            CloseHandle(handle);
        }

        if (exitDetectionThread_)
        {
            WaitForSingleObject(exitDetectionThread_, INFINITE);
        }
    }

    bool Run(Optional<_StringType> Arguments = None(), Optional<_StringType> CurrentDirectory = None(),
             Optional<DWORD> CreationFlags = None(),
             Optional<typename Win32Ex::STARTUPINFOT<_CharType>::Type *> StartupInfo = None(),
             Optional<BOOL> InheritHandles = None(), Optional<LPVOID> EnvironmentBlock = None())
    {
        if (_BasicProcess::IsRunning())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, GetLastError(), std::runtime_error("Already running"));
            return false;
        }

        return RunAsync(Arguments, CurrentDirectory, CreationFlags, StartupInfo, InheritHandles, EnvironmentBlock)
            .Wait();
    }

    virtual Waitable RunAsync(Optional<_StringType> Arguments = None(), Optional<_StringType> CurrentDirectory = None(),
                              Optional<DWORD> CreationFlags = None(),
                              Optional<typename Win32Ex::STARTUPINFOT<_CharType>::Type *> StartupInfo = None(),
                              Optional<BOOL> InheritHandles = None(), Optional<LPVOID> EnvironmentBlock = None()) = 0;

    bool Exit()
    {
        if (_BasicProcess::IsRunning())
            return TerminateProcess(_BasicProcess::processInfo_.hProcess, ERROR_PROCESS_ABORTED) == TRUE;
        return false;
    }

    //
    // Callbacks
    //

    BasicRunnableProcess &OnEnterEx(ProcessEnterCallbackWithSelf Callback)
    {
        enterCallbackWithSelf_ = Callback;
        return *this;
    }

    BasicRunnableProcess &OnExitEx(ProcessExitCallbackWithSelf Callback)
    {
        exitCallbackWithSelf_ = Callback;
        return *this;
    }

    BasicRunnableProcess &OnErrorEx(ProcessErrorCallbackWithSelf Callback)
    {
        errorCallbackWithSelf_ = Callback;
        return *this;
    }

    BasicRunnableProcess &OnEnter(ProcessEnterCallback Callback)
    {
        enterCallback_ = Callback;
        return *this;
    }

    BasicRunnableProcess &OnExit(ProcessExitCallback Callback)
    {
        exitCallback_ = Callback;
        return *this;
    }

    BasicRunnableProcess &OnError(ProcessErrorCallback Callback)
    {
        errorCallback_ = Callback;
        return *this;
    }

    bool IsAdmin() const
    {
        bool bRet = FALSE;
        HANDLE hToken;
        if (!OpenProcessToken(_BasicProcess::processInfo_.hProcess, TOKEN_QUERY | TOKEN_IMPERSONATE, &hToken))
        {
            return bRet;
        }
        if (ImpersonateLoggedOnUser(hToken))
        {
            bRet = IsUserAdmin(hToken) == TRUE;
            RevertToSelf();
        }
        CloseHandle(hToken);
        return bRet;
    }

    DWORD GetMainThreadId() const
    {
        return _BasicProcess::processInfo_.dwThreadId;
    }

    HANDLE GetMainThreadHandle() const
    {
        return _BasicProcess::processInfo_.hThread;
    }

  protected:
    _StringType arguments_;
    DWORD creationFlags_;
    BOOL inheritHandles_;
    LPVOID environmentBlock_;
    const typename Win32Ex::STARTUPINFOT<_CharType>::Type *startupInfo_;

    _StringType currentDirectory_;

    HANDLE hThreadStopEvent_;
    HANDLE exitDetectionThread_;

    ProcessEnterCallback enterCallback_;
    ProcessExitCallback exitCallback_;
    ProcessErrorCallback errorCallback_;

    ProcessEnterCallbackWithSelf enterCallbackWithSelf_;
    ProcessExitCallbackWithSelf exitCallbackWithSelf_;
    ProcessErrorCallbackWithSelf errorCallbackWithSelf_;

    //
    // WaitableObject Interface Implementations.
    //
  public:
    bool Wait(Duration Timeout)
    {
        if (WaitForSingleObject(_BasicProcess::processInfo_.hProcess, Timeout) == WAIT_OBJECT_0)
        {
            //
            //  프로세스 종료 스레드가 동작중인지 확인합니다.
            //

            if (exitDetectionThread_)
            {
                //
                //  exitDetectionThread_에서 Wait를 호출하면 안되면, 혹시나 그런 상황이 발생할때 데드락이 발생하지
                //  않도록 합니다.
                //
                if (GetCurrentThreadId() == GetThreadId(exitDetectionThread_))
                {
                    //
                    //  exitDetectionThread_ 스레드 종료 이벤트를 시그널시키고, 종료될때까지 대기합니다.
                    //
                    SetEvent(hThreadStopEvent_);
                }
                WaitForSingleObject(exitDetectionThread_, INFINITE);
            }
            return true;
        }

        return false;
    }
};

template <typename _StringType, ProcessAccountType _Type>
class BasicRunnableSessionProcess : public BasicRunnableProcess<_StringType>
{
  private:
    typedef BasicRunnableProcess<_StringType> _BasicRunnableProcess;
    typedef typename _StringType::value_type _CharType;

  public:
    BasicRunnableSessionProcess(const _StringType &ExecutablePath, const _StringType &Arguments = _StringType(),
                                const _StringType &CurrentDirectory = _StringType(), DWORD CreationFlags = 0L,
                                const typename Win32Ex::STARTUPINFOT<_CharType>::Type *StartupInfo = NULL,
                                BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL)
        : BasicRunnableProcess<_StringType>(ExecutablePath, Arguments, CurrentDirectory, CreationFlags, StartupInfo,
                                            InheritHandles, EnvironmentBlock)
    {
        _BasicRunnableProcess::sessionId_ = WTSGetActiveConsoleSessionId();
    }

    BasicRunnableSessionProcess(DWORD SessionId, const _StringType &ExecutablePath,
                                const _StringType &Arguments = _StringType(),
                                const _StringType &CurrentDirectory = _StringType(), DWORD CreationFlags = 0L,
                                const typename Win32Ex::STARTUPINFOT<_CharType>::Type *StartupInfo = NULL,
                                BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL)
        : BasicRunnableProcess<_StringType>(ExecutablePath, Arguments, CurrentDirectory, CreationFlags, StartupInfo,
                                            InheritHandles, EnvironmentBlock)
    {
        _BasicRunnableProcess::sessionId_ = SessionId;
    }

    bool IsAdmin() const
    {
        if (_Type == SystemAccount)
            return true;

        return _BasicRunnableProcess::IsAdmin();
    }

    Waitable RunAsync(Optional<_StringType> Arguments = None(), Optional<_StringType> CurrentDirectory = None(),
                      Optional<DWORD> CreationFlags = None(),
                      Optional<typename Win32Ex::STARTUPINFOT<_CharType>::Type *> StartupInfo = None(),
                      Optional<BOOL> InheritHandles = None(), Optional<LPVOID> EnvironmentBlock = None())
    {
        if (_BasicRunnableProcess::IsRunning())
        {
            if (_BasicRunnableProcess::errorCallbackWithSelf_)
                _BasicRunnableProcess::errorCallbackWithSelf_(*this, ERROR_BUSY, std::runtime_error("Already running"));
            return Waitable(*this);
        }

        if (_BasicRunnableProcess::executablePath_.empty())
        {
            if (_BasicRunnableProcess::errorCallbackWithSelf_)
                _BasicRunnableProcess::errorCallbackWithSelf_(
                    *this, ERROR_INVALID_NAME, std::runtime_error("Process image file name is not specified"));
            return Waitable(*this);
        }

        if (Arguments.IsSome())
            _BasicRunnableProcess::arguments_ = Arguments;

        if (CurrentDirectory.IsSome())
            _BasicRunnableProcess::currentDirectory_ = CurrentDirectory;

        if (CreationFlags.IsSome())
            _BasicRunnableProcess::creationFlags_ = CreationFlags;

        if (CreationFlags.IsSome())
            _BasicRunnableProcess::creationFlags_ = CreationFlags;

        if (InheritHandles.IsSome())
            _BasicRunnableProcess::inheritHandles_ = InheritHandles;

        if (EnvironmentBlock.IsSome())
            _BasicRunnableProcess::environmentBlock_ = EnvironmentBlock;

        typename Win32Ex::STARTUPINFOEXT<_CharType>::Type si;
        if (_BasicRunnableProcess::startupInfo_)
        {
#ifdef min
            CopyMemory(&si, _BasicRunnableProcess::startupInfo_,
                       min((DWORD)sizeof(si), _BasicRunnableProcess::startupInfo_->cb));
#else
            CopyMemory(&si, _BasicRunnableProcess::startupInfo_,
                       std::min((DWORD)sizeof(si), _BasicRunnableProcess::startupInfo_->cb));
#endif
        }
        else
        {
            ZeroMemory(&si, sizeof(si));
            si.StartupInfo.cb = sizeof(typename Win32Ex::STARTUPINFOT<_CharType>::Type);
        }

        _StringType command = (_BasicRunnableProcess::arguments_.empty())
                                  ? _BasicRunnableProcess::executablePath_
                                  : _BasicRunnableProcess::executablePath_ +
                                        ConstValue<typename _StringType::value_type>::Space() +
                                        _BasicRunnableProcess::arguments_;

        if (_Type == SystemAccount)
        {
            if (!CreateSystemAccountProcessT<typename _StringType::value_type>(
                    _BasicRunnableProcess::sessionId_, NULL, &command[0], NULL, NULL,
                    (InheritHandles.IsSome() ? (BOOL)InheritHandles : FALSE), _BasicRunnableProcess::creationFlags_,
                    _BasicRunnableProcess::environmentBlock_,
                    _BasicRunnableProcess::currentDirectory_.empty() ? NULL
                                                                     : _BasicRunnableProcess::currentDirectory_.c_str(),
                    &si.StartupInfo, &(_BasicRunnableProcess::processInfo_)))
            {
                if (_BasicRunnableProcess::errorCallbackWithSelf_)
                    _BasicRunnableProcess::errorCallbackWithSelf_(
                        *this, GetLastError(), std::runtime_error("Failed to CreateSystemAccountProcessT"));
                return Waitable(*this);
            }
        }
        else if (_Type == UserAccount)
        {
            if (!CreateUserAccountProcessT<typename _StringType::value_type>(
                    _BasicRunnableProcess::sessionId_, NULL, &command[0], NULL, NULL,
                    (InheritHandles.IsSome() ? (BOOL)InheritHandles : FALSE), _BasicRunnableProcess::creationFlags_,
                    _BasicRunnableProcess::environmentBlock_,
                    _BasicRunnableProcess::currentDirectory_.empty() ? NULL
                                                                     : _BasicRunnableProcess::currentDirectory_.c_str(),
                    &si.StartupInfo, &(_BasicRunnableProcess::processInfo_)))
            {
                if (_BasicRunnableProcess::errorCallbackWithSelf_)
                    _BasicRunnableProcess::errorCallbackWithSelf_(
                        *this, GetLastError(), std::runtime_error("Failed to CreateUserAccountProcessT"));
                return Waitable(*this);
            }
        }
        else
        {
            if (_BasicRunnableProcess::errorCallbackWithSelf_)
                _BasicRunnableProcess::errorCallbackWithSelf_(*this, ERROR_CLASS_DOES_NOT_EXIST,
                                                              std::runtime_error("Unknown process type"));
            return Waitable(*this);
        }

        if (_BasicRunnableProcess::enterCallbackWithSelf_)
            _BasicRunnableProcess::enterCallbackWithSelf_(*this);

        HANDLE handle = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (InterlockedCompareExchangePointer(&(_BasicRunnableProcess::hThreadStopEvent_), handle, NULL))
        {
            if (handle)
                CloseHandle(handle);
        }
        if (_BasicRunnableProcess::hThreadStopEvent_ == NULL)
        {
            if (_BasicRunnableProcess::errorCallbackWithSelf_)
                _BasicRunnableProcess::errorCallbackWithSelf_(*this, GetLastError(),
                                                              std::runtime_error("Failed to CreateEvent"));
            return Waitable(*this);
        }

        //
        //  프로세스 종료 스레드가 동작중인지 확인합니다.
        //
        if (_BasicRunnableProcess::exitDetectionThread_)
        {
            if (GetCurrentThreadId() == GetThreadId(_BasicRunnableProcess::exitDetectionThread_))
            {
                //
                //  exitDetectionThread_에서 RunAsync가 호출된것이라면, exitDetectionThread_ 스레드 객체를 분리
                //  시킵니다.
                //
                _BasicRunnableProcess::exitDetectionThread_ = NULL;
            }
            else
            {
                //
                //  exitDetectionThread_에서 RunAsync가 호출된것이 아니라면, exitDetectionThread_ 스레드 종료 이벤트를
                //  시그널시키고, 종료될때까지 대기합니다.
                //
                SetEvent(_BasicRunnableProcess::hThreadStopEvent_);
                WaitForSingleObject(_BasicRunnableProcess::exitDetectionThread_, INFINITE);
            }
        }

        _BasicRunnableProcess::exitDetectionThread_ =
            CreateThread(NULL, 0, &_BasicRunnableProcess::ExitDetectionThreadProc_, this, 0, NULL);
        return Waitable(*this);
    }
};

template <typename _StringType> class BasicElevatedProcess : public BasicRunnableProcess<_StringType>
{
  private:
    typedef BasicRunnableProcess<_StringType> _BasicRunnableProcess;
    typedef typename _StringType::value_type _CharType;

  public:
    BasicElevatedProcess(const _StringType &ExecutablePath, const _StringType &Arguments = _StringType(),
                         const _StringType &CurrentDirectory = _StringType(), DWORD CreationFlags = 0L,
                         const typename Win32Ex::STARTUPINFOT<_CharType>::Type *StartupInfo = NULL,
                         BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL)
        : BasicRunnableProcess<_StringType>(ExecutablePath, Arguments, CurrentDirectory, CreationFlags, StartupInfo,
                                            InheritHandles, EnvironmentBlock)
    {
        _BasicRunnableProcess::sessionId_ = WTSGetActiveConsoleSessionId();
    }

    Waitable RunAsync(Optional<_StringType> Arguments = None(), Optional<_StringType> CurrentDirectory = None(),
                      Optional<DWORD> CreationFlags = None(),
                      Optional<typename Win32Ex::STARTUPINFOT<_CharType>::Type *> StartupInfo = None(),
                      Optional<BOOL> InheritHandles = None(), Optional<LPVOID> EnvironmentBlock = None())
    {
        if (_BasicRunnableProcess::IsRunning())
        {
            if (_BasicRunnableProcess::errorCallbackWithSelf_)
                _BasicRunnableProcess::errorCallbackWithSelf_(*this, ERROR_BUSY, std::runtime_error("Already running"));
            return Waitable(*this);
        }

        if (_BasicRunnableProcess::executablePath_.empty())
        {
            if (_BasicRunnableProcess::errorCallbackWithSelf_)
                _BasicRunnableProcess::errorCallbackWithSelf_(
                    *this, ERROR_INVALID_NAME, std::runtime_error("Process image file name is not specified"));
            return Waitable(*this);
        }

        if (Arguments.IsSome())
            _BasicRunnableProcess::arguments_ = Arguments;

        if (CurrentDirectory.IsSome())
            _BasicRunnableProcess::currentDirectory_ = CurrentDirectory;

        if (CreationFlags.IsSome())
            _BasicRunnableProcess::creationFlags_ = CreationFlags;

        if (CreationFlags.IsSome())
            _BasicRunnableProcess::creationFlags_ = CreationFlags;

        if (InheritHandles.IsSome())
            _BasicRunnableProcess::inheritHandles_ = InheritHandles;

        if (EnvironmentBlock.IsSome())
            _BasicRunnableProcess::environmentBlock_ = EnvironmentBlock;

        typename SHELLEXECUTEINFOT<_CharType>::Type sei = {
            0,
        };
        sei.fMask |= SEE_MASK_NOCLOSEPROCESS;
        sei.cbSize = sizeof(sei);
        sei.lpVerb = ConstValue<_CharType>::RunAs();
        sei.lpFile = _BasicRunnableProcess::executablePath_.c_str();

        if (!_BasicRunnableProcess::arguments_.empty())
            sei.lpParameters = _BasicRunnableProcess::arguments_.c_str();

        if (!_BasicRunnableProcess::currentDirectory_.empty())
            sei.lpDirectory = _BasicRunnableProcess::currentDirectory_.c_str();

        if (_BasicRunnableProcess::creationFlags_ & CREATE_NO_WINDOW)
        {
            sei.fMask |= SEE_MASK_FLAG_NO_UI | SEE_MASK_NO_CONSOLE;
        }

        sei.nShow = StartupInfo.IsSome() ? StartupInfo->wShowWindow : SW_SHOWDEFAULT;

        if (!ShellExecuteExT<_CharType>(&sei))
        {
            if (_BasicRunnableProcess::errorCallbackWithSelf_)
                _BasicRunnableProcess::errorCallbackWithSelf_(*this, GetLastError(),
                                                              std::runtime_error("Failed to ShellExecuteExT"));
            return Waitable(*this);
        }

        _BasicRunnableProcess::processInfo_.dwProcessId = GetProcessId(sei.hProcess);
        _BasicRunnableProcess::processInfo_.hProcess = sei.hProcess;

        if (_BasicRunnableProcess::enterCallbackWithSelf_)
            _BasicRunnableProcess::enterCallbackWithSelf_(*this);

        HANDLE handle = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (InterlockedCompareExchangePointer(&(_BasicRunnableProcess::hThreadStopEvent_), handle, NULL))
        {
            if (handle)
                CloseHandle(handle);
        }
        if (_BasicRunnableProcess::hThreadStopEvent_ == NULL)
        {
            if (_BasicRunnableProcess::errorCallbackWithSelf_)
                _BasicRunnableProcess::errorCallbackWithSelf_(*this, GetLastError(),
                                                              std::runtime_error("Failed to CreateEvent"));
            return Waitable(*this);
        }

        //
        //  프로세스 종료 스레드가 동작중인지 확인합니다.
        //
        if (_BasicRunnableProcess::exitDetectionThread_)
        {
            if (GetCurrentThreadId() == GetThreadId(_BasicRunnableProcess::exitDetectionThread_))
            {
                //
                //  exitDetectionThread_에서 RunAsync가 호출된것이라면, exitDetectionThread_ 스레드 객체를 분리
                //  시킵니다.
                //
                _BasicRunnableProcess::exitDetectionThread_ = NULL;
            }
            else
            {
                //
                //  exitDetectionThread_에서 RunAsync가 호출된것이 아니라면, exitDetectionThread_ 스레드 종료 이벤트를
                //  시그널시키고, 종료될때까지 대기합니다.
                //
                SetEvent(_BasicRunnableProcess::hThreadStopEvent_);
                WaitForSingleObject(_BasicRunnableProcess::exitDetectionThread_, INFINITE);
            }
        }

        _BasicRunnableProcess::exitDetectionThread_ =
            CreateThread(NULL, 0, &_BasicRunnableProcess::ExitDetectionThreadProc_, this, 0, NULL);
        return Waitable(*this);
    }
}; // namespace System

typedef BasicProcess<String> Process;
typedef BasicProcess<StringW> ProcessW;

typedef BasicRunnableProcess<String> RunnableProcess;
typedef BasicRunnableProcess<StringW> RunnableProcessW;

typedef BasicElevatedProcess<String> ElevatedProcess;
typedef BasicElevatedProcess<StringW> ElevatedProcessW;

typedef BasicRunnableSessionProcess<String, UserAccount> UserAccountProcess;
typedef BasicRunnableSessionProcess<StringW, UserAccount> UserAccountProcessW;

typedef BasicRunnableSessionProcess<String, SystemAccount> SystemAccountProcess;
typedef BasicRunnableSessionProcess<StringW, SystemAccount> SystemAccountProcessW;
} // namespace System

namespace ThisProcess
{
inline System::BasicProcess<String> GetParent()
{
    return System::BasicProcess<String>(GetParentProcessId(ThisProcess::GetId()));
}
} // namespace ThisProcess
} // namespace Win32Ex

#undef _STD_NS_

#endif // _WIN32EX_SYSTEM_PROCESS_HPP_