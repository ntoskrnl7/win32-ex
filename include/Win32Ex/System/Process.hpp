/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Process.hpp

Abstract:

    This Module implements the Process, ProcessW, ProcessT /
                                ElevatedProcess, ElevatedProcessW, ElevatedProcessT /
                                UserAccountProcess, UserAccountProcessW, UserAccountProcessT /
                                SystemAccountProcess, SystemAccountProcessW, SystemAccountProcessT
                                BasicProcess, BasicRunnableProcess, BasicElevatedProcess, BasicRunnableSessionProcess
class.

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
#include <windows.h>

#include "../Internal/misc.hpp"
#include "../Optional.hpp"
#include "../Security/Token.h"
#include "../T/libloaderapi.hpp"
#include "../T/processenv.hpp"
#include "../T/processthreadsapi.hpp"
#include "../T/shellapi.hpp"
#include "../T/tlhelp32.hpp"
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
namespace System
{
namespace Details
{
namespace Process
{
template <typename CharType> struct ConstValue
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
} // namespace Process
} // namespace Details

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

template <typename _StringType> class BasicRunnableProcess;

template <typename _StringType> class BasicProcess : public WaitableObject
{
    friend class BasicRunnableProcess<_StringType>;

    WIN32EX_MOVE_ALWAYS_CLASS(BasicProcess)

  private:
    void Move(BasicProcess &To)
    {
        To.processInfo_ = processInfo_;
        To.sessionId_ = sessionId_;
        executablePath_.swap(To.executablePath_);
        ZeroMemory(&processInfo_, sizeof(processInfo_));
        sessionId_ = MAXDWORD;
        executablePath_.clear();
    }

  public:
    typedef _StringType StringType;
    typedef typename StringType::value_type CharType;

    BasicProcess(ProcessHandle ProcessHandle)
    {
        Attach_(ProcessHandle.value);
    }

    BasicProcess(ProcessId ProcessId)
    {
        HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessId);
        if (hProcess)
        {
            Attach_(hProcess);
            return;
        }

        typename PROCESSENTRY32T<CharType>::Type pe32;
        *this = GetProcessEntry_(ProcessId, pe32) ? BasicProcess(pe32) : BasicProcess();
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

    bool IsValid() const
    {
        return processInfo_.dwProcessId != 0;
    }

    bool IsAttached() const
    {
        return processInfo_.hProcess != NULL;
    }

    bool IsRunning() const
    {
        if (processInfo_.hProcess == NULL)
            return false;
        return WaitForSingleObject(processInfo_.hProcess, 0) == WAIT_TIMEOUT;
    }

    const StringType &ExecutablePath() const
    {
        return executablePath_;
    }

    DWORD Id() const
    {
        return processInfo_.dwProcessId;
    }

    HANDLE Handle() const
    {
        return processInfo_.hProcess;
    }

    BasicProcess<StringType> Parent()
    {
        DWORD parentProcessId = GetParentProcessId(processInfo_.dwProcessId);
        BasicProcess<StringType> process = BasicProcess<StringType>(parentProcessId);
        if (process.IsValid())
            return process;

        typename PROCESSENTRY32T<CharType>::Type pe32;
        return GetProcessEntry_(parentProcessId, pe32) ? BasicProcess<StringType>(pe32) : BasicProcess<StringType>();
    }

  private:
    BasicProcess() : sessionId_(MAXDWORD)
    {
        ZeroMemory(&processInfo_, sizeof(processInfo_));
    }

    BasicProcess(const typename PROCESSENTRY32T<CharType>::Type &pe32) : executablePath_(pe32.szExeFile)
    {
        if (!ProcessIdToSessionId(processInfo_.dwProcessId, &sessionId_))
            sessionId_ = MAXDWORD;
        processInfo_.dwProcessId = pe32.th32ProcessID;
        processInfo_.dwThreadId = 0;
        processInfo_.hProcess = NULL;
        processInfo_.hThread = NULL;
    }

    bool GetProcessEntry_(DWORD ProcessId, typename PROCESSENTRY32T<CharType>::Type &pe32)
    {
        HANDLE hSnapshot;
        hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE)
            return false;

        pe32.dwSize = sizeof(pe32);
        if (!Process32FirstT<CharType>(hSnapshot, &pe32))
        {
            CloseHandle(hSnapshot);
            return false;
        }

        do
        {
            if (pe32.th32ProcessID == ProcessId)
            {
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32NextT<CharType>(hSnapshot, &pe32));

        CloseHandle(hSnapshot);
        return false;
    }

  private:
    void Attach_(HANDLE ProcessHandle)
    {
        DWORD processId = GetProcessId(ProcessHandle);
        DWORD length = MAX_PATH;
        StringType executablePath;
        executablePath.resize(length);
        while (!QueryFullProcessImageNameT<CharType>(ProcessHandle, 0, &executablePath[0], &length))
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
        else
        {
            typename PROCESSENTRY32T<CharType>::Type pe32;
            if (GetProcessEntry_(processId, pe32))
                executablePath_ = pe32.szExeFile;
        }

        if (!ProcessIdToSessionId(processId, &sessionId_))
            sessionId_ = MAXDWORD;

        processInfo_.dwProcessId = processId;
        processInfo_.dwThreadId = 0;
        processInfo_.hProcess = ProcessHandle;
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

  protected:
    DWORD sessionId_;
    StringType executablePath_;
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
    friend class BasicRunnableSessionProcess<_StringType, SystemAccount>;
    friend class BasicRunnableSessionProcess<_StringType, UserAccount>;
    friend class BasicElevatedProcess<_StringType>;
    friend class BasicElevatedProcess<_StringType>;

  public:
    typedef _StringType StringType;
    typedef typename StringType::value_type CharType;

  private:
    typedef BasicProcess<StringType> _BasicProcess;

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
    BasicRunnableProcess(const StringType &ExecutablePath, const Optional<const StringType &> &Arguments = None(),
                         const Optional<const StringType &> &CurrentDirectory = None())
        : _BasicProcess(), exitDetectionThread_(NULL), hThreadStopEvent_(NULL), arguments_(Arguments),
          currentDirectory_(CurrentDirectory)
    {
        _BasicProcess::executablePath_ = ExecutablePath;
        InitCallbacks_();
    }

    bool Prepare()
    {
        if (_BasicProcess::IsRunning())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, ERROR_BUSY, std::runtime_error("Already running"));
            return false;
        }

        if (_BasicProcess::executablePath_.empty())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, ERROR_INVALID_NAME,
                                       std::runtime_error("Process image file name is not specified"));
            return false;
        }
        return true;
    }

    bool Finish()
    {
        if (enterCallbackWithSelf_)
            enterCallbackWithSelf_(*this);

        HANDLE handle = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (InterlockedCompareExchangePointer(&(hThreadStopEvent_), handle, NULL))
        {
            if (handle)
                CloseHandle(handle);
        }
        if (hThreadStopEvent_ == NULL)
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, GetLastError(), std::runtime_error("Failed to CreateEvent"));
            return false;
        }

        //
        //  프로세스 종료 스레드가 동작중인지 확인합니다.
        //
        if (exitDetectionThread_)
        {
            if (GetCurrentThreadId() == GetThreadId(exitDetectionThread_))
            {
                //
                //  exitDetectionThread_에서 RunAsync가 호출된것이라면, exitDetectionThread_ 스레드 객체를 분리
                //  시킵니다.
                //
                exitDetectionThread_ = NULL;
            }
            else
            {
                //
                //  exitDetectionThread_에서 RunAsync가 호출된것이 아니라면, exitDetectionThread_ 스레드 종료 이벤트를
                //  시그널시키고, 종료될때까지 대기합니다.
                //
                SetEvent(hThreadStopEvent_);
                WaitForSingleObject(exitDetectionThread_, INFINITE);
            }
        }

        exitDetectionThread_ = CreateThread(NULL, 0, &ExitDetectionThreadProc_, this, 0, NULL);
        return exitDetectionThread_ != NULL;
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

    bool Exit()
    {
        if (_BasicProcess::IsRunning())
            return TerminateProcess(_BasicProcess::processInfo_.hProcess, ERROR_PROCESS_ABORTED) == TRUE;
        return false;
    }

    bool IsAdmin() const
    {
        HANDLE hToken;
        if (!OpenProcessToken(_BasicProcess::processInfo_.hProcess, TOKEN_QUERY | TOKEN_IMPERSONATE, &hToken))
            return FALSE;

        bool bRet = FALSE;
        if (ImpersonateLoggedOnUser(hToken))
        {
            bRet = IsUserAdmin(hToken) == TRUE;
            RevertToSelf();
        }
        CloseHandle(hToken);
        return bRet;
    }

    bool Run()
    {
        return RunAsync().Wait();
    }

  public:
    virtual Waitable RunAsync() = 0;

  public:
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

  protected:
    Optional<StringType> arguments_;
    Optional<StringType> currentDirectory_;

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

  public:
    typedef _StringType StringType;
    typedef typename StringType::value_type CharType;

    BasicRunnableSessionProcess(const StringType &ExecutablePath,
                                const Optional<const StringType &> &Arguments = None(),
                                const Optional<const StringType &> &CurrentDirectory = None(), DWORD CreationFlags = 0L,
                                const typename Win32Ex::STARTUPINFOT<CharType>::Type *StartupInfo = NULL,
                                BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL)
        : BasicRunnableProcess<StringType>(ExecutablePath, Arguments, CurrentDirectory), creationFlags_(CreationFlags),
          startupInfo_(StartupInfo), inheritHandles_(InheritHandles), environmentBlock_(EnvironmentBlock)
    {
        _BasicRunnableProcess::sessionId_ = WTSGetActiveConsoleSessionId();
    }

    BasicRunnableSessionProcess(DWORD SessionId, const StringType &ExecutablePath,
                                const Optional<const StringType &> &Arguments = None(),
                                const Optional<const StringType &> &CurrentDirectory = None(), DWORD CreationFlags = 0L,
                                const typename Win32Ex::STARTUPINFOT<CharType>::Type *StartupInfo = NULL,
                                BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL)
        : BasicRunnableProcess<StringType>(ExecutablePath, Arguments, CurrentDirectory), creationFlags_(CreationFlags),
          startupInfo_(StartupInfo), inheritHandles_(InheritHandles), environmentBlock_(EnvironmentBlock)
    {
        _BasicRunnableProcess::sessionId_ = SessionId;
    }

    bool IsAdmin() const
    {
        if (_Type == SystemAccount)
            return true;

        return _BasicRunnableProcess::IsAdmin();
    }

    DWORD MainThreadId() const
    {
        return _BasicRunnableProcess::processInfo_.dwThreadId;
    }

    HANDLE MainThreadHandle() const
    {
        return _BasicRunnableProcess::processInfo_.hThread;
    }

    bool Run(const Optional<DWORD> &SessionId = None(), const Optional<const StringType &> &Arguments = None(),
             const Optional<const StringType &> &CurrentDirectory = None(), Optional<DWORD> CreationFlags = None(),
             const Optional<typename Win32Ex::STARTUPINFOT<CharType>::Type *> &StartupInfo = None(),
             const Optional<BOOL> &InheritHandles = None(), const Optional<LPVOID> &EnvironmentBlock = None())
    {
        if (SessionId.IsSome())
            _BasicRunnableProcess::sessionId_ = SessionId;

        return RunAsync(_BasicRunnableProcess::sessionId_, Arguments, CurrentDirectory, CreationFlags, StartupInfo,
                        InheritHandles, EnvironmentBlock)
            .Wait();
    }

    Waitable RunAsync(DWORD SessionId, const Optional<const StringType &> &Arguments = None(),
                      const Optional<const StringType &> &CurrentDirectory = None(),
                      const Optional<DWORD> &CreationFlags = None(),
                      const Optional<typename Win32Ex::STARTUPINFOT<CharType>::Type *> &StartupInfo = None(),
                      const Optional<BOOL> &InheritHandles = None(), const Optional<LPVOID> &EnvironmentBlock = None())
    {
        if (!_BasicRunnableProcess::Prepare())
            return Waitable(*this);

        if (_BasicRunnableProcess::sessionId_ != SessionId)
            _BasicRunnableProcess::sessionId_ = SessionId;

        if (Arguments.IsSome())
            _BasicRunnableProcess::arguments_ = Arguments;

        if (CurrentDirectory.IsSome())
            _BasicRunnableProcess::currentDirectory_ = CurrentDirectory;

        if (CreationFlags.IsSome())
            creationFlags_ = CreationFlags;

        if (StartupInfo.IsSome())
            startupInfo_ = StartupInfo;

        if (InheritHandles.IsSome())
            inheritHandles_ = InheritHandles;

        if (EnvironmentBlock.IsSome())
            environmentBlock_ = EnvironmentBlock;

        typename Win32Ex::STARTUPINFOEXT<CharType>::Type si;
        if (startupInfo_)
        {
#ifdef min
            CopyMemory(&si, startupInfo_, min((DWORD)sizeof(si), startupInfo_->cb));
#else
            CopyMemory(&si, startupInfo_, std::min((DWORD)sizeof(si), startupInfo_->cb));
#endif
        }
        else
        {
            ZeroMemory(&si, sizeof(si));
            si.StartupInfo.cb = sizeof(typename Win32Ex::STARTUPINFOT<CharType>::Type);
        }

        StringType command = _BasicRunnableProcess::executablePath_;
        if (_BasicRunnableProcess::arguments_.IsSome())
        {
            command += Details::Process::ConstValue<CharType>::Space();
            command += _BasicRunnableProcess::arguments_;
        }

        if (_Type == SystemAccount)
        {
            if (!CreateSystemAccountProcessT<CharType>(
                    _BasicRunnableProcess::sessionId_, NULL, &command[0], NULL, NULL,
                    (InheritHandles.IsSome() ? (BOOL)InheritHandles : FALSE), creationFlags_, environmentBlock_,
                    _BasicRunnableProcess::currentDirectory_.IsSome() ? _BasicRunnableProcess::currentDirectory_.Get()
                                                                      : NULL,
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
            if (!CreateUserAccountProcessT<CharType>(
                    _BasicRunnableProcess::sessionId_, NULL, &command[0], NULL, NULL,
                    (InheritHandles.IsSome() ? (BOOL)InheritHandles : FALSE), creationFlags_, environmentBlock_,
                    _BasicRunnableProcess::currentDirectory_.IsSome() ? _BasicRunnableProcess::currentDirectory_.Get()
                                                                      : NULL,
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

        _BasicRunnableProcess::Finish();

        return Waitable(*this);
    }

  private:
    DWORD creationFlags_;
    BOOL inheritHandles_;
    LPVOID environmentBlock_;
    const typename Win32Ex::STARTUPINFOT<CharType>::Type *startupInfo_;

    //
    // BasicRunnableProcess Interface Implementations.
    //
  public:
    virtual Waitable RunAsync()
    {
        return RunAsync(_BasicRunnableProcess::sessionId_);
    }
};

template <typename _StringType> class BasicElevatedProcess : public BasicRunnableProcess<_StringType>
{
  private:
    typedef BasicRunnableProcess<_StringType> _BasicRunnableProcess;

  public:
    typedef _StringType StringType;
    typedef typename StringType::value_type CharType;

    BasicElevatedProcess(const StringType &ExecutablePath, const Optional<const StringType &> &Arguments = None(),
                         const Optional<const StringType &> &CurrentDirectory = None(), DWORD CmdShow = SW_SHOWDEFAULT)
        : _BasicRunnableProcess(ExecutablePath, Arguments, CurrentDirectory)
    {
        ZeroMemory(&executeInfo_, sizeof(executeInfo_));

        executeInfo_.cbSize = sizeof(executeInfo_);
        executeInfo_.fMask |= SEE_MASK_NOCLOSEPROCESS;
        executeInfo_.lpVerb = Details::Process::ConstValue<CharType>::RunAs();
        executeInfo_.lpFile = _BasicRunnableProcess::executablePath_.c_str();

        if (_BasicRunnableProcess::arguments_.IsSome())
            executeInfo_.lpParameters = _BasicRunnableProcess::arguments_.Get();
        if (_BasicRunnableProcess::currentDirectory_.IsSome())
            executeInfo_.lpDirectory = _BasicRunnableProcess::currentDirectory_.Get();

        executeInfo_.nShow = CmdShow;
        if (CmdShow == SW_HIDE)
            executeInfo_.fMask |= SEE_MASK_FLAG_NO_UI | SEE_MASK_NO_CONSOLE;

        _BasicRunnableProcess::sessionId_ = WTSGetActiveConsoleSessionId();
    }

    BasicElevatedProcess(typename SHELLEXECUTEINFOT<CharType>::Type &ExecuteInfo)
        : _BasicRunnableProcess(ExecuteInfo.lpFile, ExecuteInfo.lpParameters, ExecuteInfo.lpDirectory)
    {
        executeInfo_ = ExecuteInfo;
        _BasicRunnableProcess::sessionId_ = WTSGetActiveConsoleSessionId();
    }

    bool Run(const typename SHELLEXECUTEINFOT<CharType>::Type &ExecuteInfo)
    {
        return RunAsync(ExecuteInfo).Wait();
    }

    bool Run(const Optional<const StringType &> &Arguments = None(),
             const Optional<const StringType &> &CurrentDirectory = None(), const Optional<DWORD> &CmdShow = None())
    {
        if (Arguments.IsSome())
            _BasicRunnableProcess::arguments_ = Arguments;

        return RunAsync(_BasicRunnableProcess::arguments_, CurrentDirectory, CmdShow).Wait();
    }

    Waitable RunAsync(const Optional<StringType> &Arguments,
                      const Optional<const StringType &> &CurrentDirectory = None(),
                      const Optional<DWORD> &CmdShow = None())
    {
        if (!_BasicRunnableProcess::Prepare())
            return Waitable(*this);

        executeInfo_.cbSize = sizeof(executeInfo_);
        executeInfo_.fMask |= SEE_MASK_NOCLOSEPROCESS;
        executeInfo_.lpVerb = Details::Process::ConstValue<CharType>::RunAs();
        executeInfo_.lpFile = _BasicRunnableProcess::executablePath_.c_str();

        if (Arguments.IsSome())
        {
            _BasicRunnableProcess::arguments_ = Arguments.Clone();
            executeInfo_.lpParameters = _BasicRunnableProcess::arguments_.Get();
        }

        if (CurrentDirectory.IsSome())
        {
            _BasicRunnableProcess::currentDirectory_ = CurrentDirectory.Clone();
            executeInfo_.lpDirectory = _BasicRunnableProcess::currentDirectory_.Get();
        }

        if (CmdShow.IsSome())
        {
            executeInfo_.nShow = CmdShow;
            if (CmdShow == SW_HIDE)
                executeInfo_.fMask |= SEE_MASK_FLAG_NO_UI | SEE_MASK_NO_CONSOLE;
        }

        return RunAsync();
    }

    Waitable RunAsync(typename SHELLEXECUTEINFOT<CharType>::Type &ExecuteInfo)
    {
        if (!_BasicRunnableProcess::Prepare())
            return Waitable(*this);

        if (&ExecuteInfo != &executeInfo_)
        {
            _BasicRunnableProcess::executablePath_ = ExecuteInfo.lpFile;
            _BasicRunnableProcess::arguments_ = ExecuteInfo.lpParameters;
            _BasicRunnableProcess::currentDirectory_ = ExecuteInfo.lpDirectory;
            executeInfo_ = ExecuteInfo;
        }

        if (!ShellExecuteExT<CharType>(&ExecuteInfo))
        {
            if (_BasicRunnableProcess::errorCallbackWithSelf_)
                _BasicRunnableProcess::errorCallbackWithSelf_(*this, GetLastError(),
                                                              std::runtime_error("Failed to ShellExecuteExT"));
            return Waitable(*this);
        }

        _BasicRunnableProcess::processInfo_.dwProcessId = GetProcessId(ExecuteInfo.hProcess);
        _BasicRunnableProcess::processInfo_.hProcess = ExecuteInfo.hProcess;

        _BasicRunnableProcess::Finish();

        return Waitable(*this);
    }

  private:
    typename SHELLEXECUTEINFOT<CharType>::Type executeInfo_;

    //
    // BasicRunnableProcess Interface Implementations.
    //
  public:
    virtual Waitable RunAsync()
    {
        return RunAsync(executeInfo_);
    }
};

typedef BasicProcess<String> Process;
typedef BasicProcess<StringW> ProcessW;
typedef BasicProcess<StringT> ProcessT;

typedef BasicRunnableProcess<String> RunnableProcess;
typedef BasicRunnableProcess<StringW> RunnableProcessW;
typedef BasicRunnableProcess<StringT> RunnableProcessT;

typedef BasicElevatedProcess<String> ElevatedProcess;
typedef BasicElevatedProcess<StringW> ElevatedProcessW;
typedef BasicElevatedProcess<StringT> ElevatedProcessT;

typedef BasicRunnableSessionProcess<String, UserAccount> UserAccountProcess;
typedef BasicRunnableSessionProcess<StringW, UserAccount> UserAccountProcessW;
typedef BasicRunnableSessionProcess<StringT, UserAccount> UserAccountProcessT;

typedef BasicRunnableSessionProcess<String, SystemAccount> SystemAccountProcess;
typedef BasicRunnableSessionProcess<StringW, SystemAccount> SystemAccountProcessW;
typedef BasicRunnableSessionProcess<StringT, SystemAccount> SystemAccountProcessT;
} // namespace System

namespace ThisProcess
{
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

static DWORD MainThreadId()
{
    return Details::mainThreadId;
}

static HANDLE MainThreadHandle()
{
    return Details::mainThreadHandle;
}

static DWORD Id()
{
    return GetCurrentProcessId();
}

static HANDLE Handle()
{
    return GetCurrentProcess();
}

static bool IsAdmin()
{
    return IsUserAdmin(GetCurrentProcessToken()) == TRUE;
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class _StringType = StringT>
#else
template <class _StringType>
#endif
static _StringType &ExecutablePathT()
{
    static _StringType executablePath_(MAX_PATH, 0);
    size_t returnSize =
        GetModuleFileNameT<typename _StringType::value_type>(NULL, &executablePath_[0], (DWORD)executablePath_.size());
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
            returnSize = GetModuleFileNameT<typename _StringType::value_type>(NULL, &executablePath_[0],
                                                                              (DWORD)executablePath_.size());
            executablePath_.resize(returnSize);
        }
        if (GetLastError() == ERROR_SUCCESS)
            break;
    }
    return executablePath_;
}
static String ExecutablePath()
{
    return ExecutablePathT<String>();
}
static StringW ExecutablePathW()
{
    return ExecutablePathT<StringW>();
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class _StringType = StringT>
#else
template <class _StringType>
#endif
static _StringType CurrentDirectoryT()
{
    _StringType cwd(MAX_PATH, 0);
    size_t length = Win32Ex::GetCurrentDirectoryT<typename _StringType::value_type>((DWORD)cwd.size(), &cwd[0]);
    if (length > cwd.size())
    {
        cwd.resize(length);
        length = Win32Ex::GetCurrentDirectoryT<typename _StringType::value_type>((DWORD)cwd.size(), &cwd[0]);
    }
    cwd.resize(length);
    return cwd;
}
static String CurrentDirectory()
{
    return CurrentDirectoryT<String>();
}
static StringW CurrentDirectoryW()
{
    return CurrentDirectoryT<StringW>();
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class _StringType = StringT>
#else
template <class _StringType>
#endif
inline System::BasicProcess<_StringType> ParentT()
{
    return System::BasicProcess<_StringType>(GetParentProcessId(ThisProcess::Id()));
}
inline System::Process Parent()
{
    return System::Process(GetParentProcessId(ThisProcess::Id()));
}
inline System::ProcessW ParentW()
{
    return System::ProcessW(GetParentProcessId(ThisProcess::Id()));
}
} // namespace ThisProcess
} // namespace Win32Ex

#undef _STD_NS_

#endif // _WIN32EX_SYSTEM_PROCESS_HPP_