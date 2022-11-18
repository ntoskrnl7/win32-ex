/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Process.hpp

Abstract:

    This Module implements the Process, ProcessW, ProcessT /
                                ElevatedProcess, ElevatedProcessW, ElevatedProcessT /
                                UserAccountProcess, UserAccountProcessW, UserAccountProcessT /
                                SystemAccountProcess, SystemAccountProcessW, SystemAccountProcessT /
                                RunnableSessionProcessT class.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SYSTEM_PROCESS_HPP_
#define _WIN32EX_SYSTEM_PROCESS_HPP_

#if _MSC_VER <= 1600
#pragma warning(disable : 4481 4714)
#endif

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
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>

#if defined(__GLIBCXX__)
#include <io.h>
#endif

#include <ext/pipe>

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

template <typename StringType> class RunnableProcessT;

template <typename StringType = StringT> class ProcessT : public WaitableObject
{
    friend class RunnableProcessT<StringType>;

    WIN32EX_MOVE_ONLY_CLASS(ProcessT)

  private:
    void Move(ProcessT &To)
    {
        To.processInfo_ = processInfo_;
        To.sessionId_ = sessionId_;
        executablePath_.swap(To.executablePath_);
        ZeroMemory(&processInfo_, sizeof(processInfo_));
        sessionId_ = MAXDWORD;
        executablePath_.clear();
    }

  public:
    ProcessT Clone() const
    {
        ProcessT clone;
        clone.processInfo_.dwProcessId = processInfo_.dwProcessId;
        if (processInfo_.hProcess)
        {
            DuplicateHandle(GetCurrentProcess(), processInfo_.hProcess, GetCurrentProcess(),
                            &clone.processInfo_.hProcess, 0, FALSE, DUPLICATE_SAME_ACCESS);
        }

        clone.processInfo_.dwThreadId = processInfo_.dwThreadId;
        if (processInfo_.hThread)
        {
            DuplicateHandle(GetCurrentProcess(), processInfo_.hThread, GetCurrentProcess(), &clone.processInfo_.hThread,
                            0, FALSE, DUPLICATE_SAME_ACCESS);
        }
        clone.executablePath_ = executablePath_;
        clone.sessionId_ = sessionId_;
        return clone;
    }

  public:
    typedef typename StringType::value_type CharType;

    ProcessT(ProcessHandle ProcessHandle)
    {
        Attach_(ProcessHandle.value);
    }

    ProcessT(ProcessId ProcessId)
    {
        HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessId);
        if (hProcess)
        {
            Attach_(hProcess);
            return;
        }

        typename PROCESSENTRY32T<CharType>::Type pe32;
        *this = GetProcessEntry_(ProcessId, pe32) ? ProcessT(pe32) : ProcessT();
    }

    ~ProcessT()
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

    ProcessT<StringType> Parent()
    {
        DWORD parentProcessId = GetParentProcessId(processInfo_.dwProcessId);
        ProcessT<StringType> process = ProcessT<StringType>(parentProcessId);
        if (process.IsValid())
            return process;

        typename PROCESSENTRY32T<CharType>::Type pe32;
        return GetProcessEntry_(parentProcessId, pe32) ? ProcessT<StringType>(pe32) : ProcessT<StringType>();
    }

  private:
    ProcessT() : sessionId_(MAXDWORD)
    {
        ZeroMemory(&processInfo_, sizeof(processInfo_));
    }

    ProcessT(const typename PROCESSENTRY32T<CharType>::Type &pe32) : executablePath_(pe32.szExeFile)
    {
        processInfo_.dwProcessId = pe32.th32ProcessID;
        processInfo_.dwThreadId = 0;
        processInfo_.hProcess = NULL;
        processInfo_.hThread = NULL;
        if (!ProcessIdToSessionId(processInfo_.dwProcessId, &sessionId_))
            sessionId_ = MAXDWORD;
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
    bool IsWaitable() override
    {
        return processInfo_.hProcess != NULL;
    }

    bool Wait(Duration Timeout) override
    {
        return WaitForSingleObject(processInfo_.hProcess, Timeout) == WAIT_OBJECT_0;
    }

  public:
    static std::list<SharedPtr<ProcessT>> All()
    {
        std::list<SharedPtr<ProcessT>> all;

        HANDLE hSnapshot;
        hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE)
            return std::list<SharedPtr<ProcessT>>();

        PROCESSENTRY32T<CharType> pe32;
        pe32.dwSize = sizeof(pe32);
        if (!Process32FirstT<CharType>(hSnapshot, &pe32))
        {
            CloseHandle(hSnapshot);
            return std::list<SharedPtr<ProcessT>>();
        }

        do
        {
#if defined(_MSC_VER) && _MSC_VER < 1600
            all.push_back(SharedPtr<ProcessT>(new ProcessT(pe32.th32ProcessID)));
#else
            all.push_back(std::make_shared<ProcessT>(pe32.th32ProcessID));
#endif
        } while (Process32NextT<CharType>(hSnapshot, &pe32));

        CloseHandle(hSnapshot);
        return all;
    }
};

enum ProcessAccountType
{
    SystemAccount,
    UserAccount
};

template <ProcessAccountType Type, typename StringType> class RunnableSessionProcessT;
template <typename StringType> class ElevatedProcessT;

template <typename StringType = StringT> class RunnableProcessT : public ProcessT<StringType>
{
    friend class RunnableSessionProcessT<SystemAccount, StringType>;
    friend class RunnableSessionProcessT<UserAccount, StringType>;
    friend class ElevatedProcessT<StringType>;
    friend class ElevatedProcessT<StringType>;

  public:
    typedef typename StringType::value_type CharType;

  private:
    typedef ProcessT<StringType> _Process;

    typedef std::function<void(RunnableProcessT &)> ProcessEnterCallbackWithSelf;
    typedef std::function<void(RunnableProcessT &)> ProcessExitCallbackWithSelf;
    typedef std::function<void(RunnableProcessT &, DWORD, const std::exception &)> ProcessErrorCallbackWithSelf;

    typedef std::function<void()> ProcessEnterCallback;
    typedef std::function<void()> ProcessExitCallback;
    typedef std::function<void(DWORD, const std::exception &)> ProcessErrorCallback;

    void InitCallbacks_()
    {
        enterCallbackWithSelf_ =
            _STD_NS_::bind(&RunnableProcessT::EnterCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1);
        exitCallbackWithSelf_ =
            _STD_NS_::bind(&RunnableProcessT::ExitCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1);
        errorCallbackWithSelf_ =
            _STD_NS_::bind(&RunnableProcessT::ErrorCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1,
                           _STD_NS_::placeholders::_2, _STD_NS_::placeholders::_3);
    }

    void EnterCallbackWithSelfDefault_(RunnableProcessT &)
    {
        if (enterCallback_)
            enterCallback_();
    };

    void ExitCallbackWithSelfDefault_(RunnableProcessT &)
    {
        if (exitCallback_)
            exitCallback_();
    };

    void ErrorCallbackWithSelfDefault_(RunnableProcessT &, DWORD LastError, const std::exception &e)
    {
        if (errorCallback_)
            errorCallback_(LastError, e);
    };

    static DWORD WINAPI ExitDetectionThreadProc_(PVOID lpThreadParameter)
    {
        RunnableProcessT *self = (RunnableProcessT *)lpThreadParameter;
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
    RunnableProcessT(const StringType &ExecutablePath, const Optional<const StringType &> &Arguments = None(),
                     const Optional<const StringType &> &CurrentDirectory = None())
        : _Process(), exitDetectionThread_(NULL), hThreadStopEvent_(NULL), arguments_(Arguments),
          currentDirectory_(CurrentDirectory)
    {
        _Process::executablePath_ = ExecutablePath;
        InitCallbacks_();
    }

    bool Prepare()
    {
        if (_Process::IsRunning())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, ERROR_BUSY, std::runtime_error("Already running"));
            return false;
        }

        if (_Process::executablePath_.empty())
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
    ~RunnableProcessT()
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
        if (_Process::IsRunning())
            return TerminateProcess(_Process::processInfo_.hProcess, ERROR_PROCESS_ABORTED) == TRUE;
        return false;
    }

    bool IsAdmin() const
    {
        HANDLE hToken;
        if (!OpenProcessToken(_Process::processInfo_.hProcess, TOKEN_QUERY | TOKEN_IMPERSONATE, &hToken))
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
    RunnableProcessT &OnEnterEx(ProcessEnterCallbackWithSelf Callback)
    {
        enterCallbackWithSelf_ = Callback;
        return *this;
    }

    RunnableProcessT &OnExitEx(ProcessExitCallbackWithSelf Callback)
    {
        exitCallbackWithSelf_ = Callback;
        return *this;
    }

    RunnableProcessT &OnErrorEx(ProcessErrorCallbackWithSelf Callback)
    {
        errorCallbackWithSelf_ = Callback;
        return *this;
    }

    RunnableProcessT &OnEnter(ProcessEnterCallback Callback)
    {
        enterCallback_ = Callback;
        return *this;
    }

    RunnableProcessT &OnExit(ProcessExitCallback Callback)
    {
        exitCallback_ = Callback;
        return *this;
    }

    RunnableProcessT &OnError(ProcessErrorCallback Callback)
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
    bool Wait(Duration Timeout) override
    {
        if (WaitForSingleObject(_Process::processInfo_.hProcess, Timeout) == WAIT_OBJECT_0)
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

template <ProcessAccountType Type, typename StringType = StringT>
class RunnableSessionProcessT : public RunnableProcessT<StringType>
{
  private:
    typedef RunnableProcessT<StringType> _RunnableProcess;

  public:
    typedef typename StringType::value_type CharType;

    RunnableSessionProcessT(const StringType &ExecutablePath, const Optional<const StringType &> &Arguments = None(),
                            const Optional<const StringType &> &CurrentDirectory = None(), DWORD CreationFlags = 0L,
                            const typename Win32Ex::STARTUPINFOT<CharType>::Type *StartupInfo = NULL,
                            BOOL InheritHandles = TRUE, LPVOID EnvironmentBlock = NULL)
        : RunnableProcessT<StringType>(ExecutablePath, Arguments, CurrentDirectory), creationFlags_(CreationFlags),
          startupInfo_(StartupInfo), inheritHandles_(InheritHandles), environmentBlock_(EnvironmentBlock)
    {
        _RunnableProcess::sessionId_ = WTSGetActiveConsoleSessionId();
    }

    RunnableSessionProcessT(DWORD SessionId, const StringType &ExecutablePath,
                            const Optional<const StringType &> &Arguments = None(),
                            const Optional<const StringType &> &CurrentDirectory = None(), DWORD CreationFlags = 0L,
                            const typename Win32Ex::STARTUPINFOT<CharType>::Type *StartupInfo = NULL,
                            BOOL InheritHandles = TRUE, LPVOID EnvironmentBlock = NULL)
        : RunnableProcessT<StringType>(ExecutablePath, Arguments, CurrentDirectory), creationFlags_(CreationFlags),
          startupInfo_(StartupInfo), inheritHandles_(InheritHandles), environmentBlock_(EnvironmentBlock)
    {
        _RunnableProcess::sessionId_ = SessionId;
    }

    bool IsAdmin() const
    {
        if (Type == SystemAccount)
            return true;

        return _RunnableProcess::IsAdmin();
    }

    DWORD MainThreadId() const
    {
        return _RunnableProcess::processInfo_.dwThreadId;
    }

    HANDLE MainThreadHandle() const
    {
        return _RunnableProcess::processInfo_.hThread;
    }

    bool Run(const Optional<const StringType &> &Arguments = None(),
             const Optional<const StringType &> &CurrentDirectory = None(), Optional<DWORD> CreationFlags = None(),
             const Optional<typename Win32Ex::STARTUPINFOT<CharType>::Type *> &StartupInfo = None(),
             const Optional<BOOL> &InheritHandles = None(), const Optional<LPVOID> &EnvironmentBlock = None())
    {
        return RunAsync(Arguments, CurrentDirectory, CreationFlags, StartupInfo, InheritHandles, EnvironmentBlock)
            .Wait();
    }
    void test(const ext::opstream &xx)
    {
    }
    void test(ext::opstream &xx)
    {
    }
    void test(ext::opstream &&xx)
    {
    }
    Waitable RunAsync(const Optional<const StringType &> &Arguments,
                      const Optional<const StringType &> &CurrentDirectory = None(),
                      const Optional<DWORD> &CreationFlags = None(),
                      const Optional<typename Win32Ex::STARTUPINFOT<CharType>::Type *> &StartupInfo = None(),
                      const Optional<BOOL> &InheritHandles = None(), const Optional<LPVOID> &EnvironmentBlock = None())
    {
        if (!_RunnableProcess::Prepare())
            return Waitable(*this);

        if (Arguments.IsSome())
            _RunnableProcess::arguments_ = Arguments;

        if (CurrentDirectory.IsSome())
            _RunnableProcess::currentDirectory_ = CurrentDirectory;

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

        StringType command = _RunnableProcess::executablePath_;
        if (_RunnableProcess::arguments_.IsSome())
        {
            command += Details::Process::ConstValue<CharType>::Space();
            command += _RunnableProcess::arguments_;
        }

        ext::pipe in;
        ext::pipe out;
        ext::pipe err;
        si.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
#ifdef __cpp_rvalue_references
        in_ = std::move(in.out());
        out_ = std::move(out.in());
        err_ = std::move(err.in());
#else
        in_.swap(in.out());
        out_.swap(out.in());
        err_.swap(err.in());
#endif // __cpp_rvalue_references
#if defined(__GLIBCXX__)
        si.StartupInfo.hStdInput = (HANDLE)_get_osfhandle(in.in().native_handle());
        si.StartupInfo.hStdOutput = (HANDLE)_get_osfhandle(out.out().native_handle());
        si.StartupInfo.hStdError = (HANDLE)_get_osfhandle(err.out().native_handle());
        SetHandleInformation((HANDLE)_get_osfhandle(in_.native_handle()), HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation((HANDLE)_get_osfhandle(out_.native_handle()), HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation((HANDLE)_get_osfhandle(err_.native_handle()), HANDLE_FLAG_INHERIT, 0);
#else
        si.StartupInfo.hStdInput = in.in().native_handle();
        si.StartupInfo.hStdOutput = out.out().native_handle();
        si.StartupInfo.hStdError = err.out().native_handle();
        SetHandleInformation(in_.native_handle(), HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(out_.native_handle(), HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(err_.native_handle(), HANDLE_FLAG_INHERIT, 0);
#endif // defined(__GLIBCXX__)
        if (Type == SystemAccount)
        {
            if (!CreateSystemAccountProcessT<CharType>(
                    _RunnableProcess::sessionId_, NULL, &command[0], NULL, NULL, inheritHandles_, creationFlags_,
                    environmentBlock_,
                    _RunnableProcess::currentDirectory_.IsSome() ? _RunnableProcess::currentDirectory_.Get() : NULL,
                    &si.StartupInfo, &(_RunnableProcess::processInfo_)))
            {
                if (_RunnableProcess::errorCallbackWithSelf_)
                    _RunnableProcess::errorCallbackWithSelf_(
                        *this, GetLastError(), std::runtime_error("Failed to CreateSystemAccountProcessT"));
                return Waitable(*this);
            }
        }
        else if (Type == UserAccount)
        {
            if (!CreateUserAccountProcessT<CharType>(
                    _RunnableProcess::sessionId_, NULL, &command[0], NULL, NULL, inheritHandles_, creationFlags_,
                    environmentBlock_,
                    _RunnableProcess::currentDirectory_.IsSome() ? _RunnableProcess::currentDirectory_.Get() : NULL,
                    &si.StartupInfo, &(_RunnableProcess::processInfo_)))
            {
                if (_RunnableProcess::errorCallbackWithSelf_)
                    _RunnableProcess::errorCallbackWithSelf_(*this, GetLastError(),
                                                             std::runtime_error("Failed to CreateUserAccountProcessT"));
                return Waitable(*this);
            }
        }
        else
        {
            if (_RunnableProcess::errorCallbackWithSelf_)
                _RunnableProcess::errorCallbackWithSelf_(*this, ERROR_CLASS_DOES_NOT_EXIST,
                                                         std::runtime_error("Unknown process type"));
            return Waitable(*this);
        }

        _RunnableProcess::Finish();

        return Waitable(*this);
    }

    class OutputStream : public ext::opstream
    {
      public:
        OutputStream()
        {
        }
#ifdef __cpp_rvalue_references
        OutputStream(ext::opstream &&stream) : ext::opstream(std::move(stream))
        {
        }

        OutputStream &operator=(ext::opstream &&rhs)
        {
            swap(rhs);
            return *this;
        }
#endif
        void Close()
        {
            ext::opstream::close();
        }
    };

    class InputStream : public ext::ipstream
    {
      public:
        InputStream()
        {
        }
#ifdef __cpp_rvalue_references
        InputStream(ext::ipstream &&stream) : ext::ipstream(std::move(stream))
        {
        }

        InputStream &operator=(ext::ipstream &&rhs)
        {
            swap(rhs);
            return *this;
        }
#endif
        std::string ReadAll() const
        {
            return std::string((std::istreambuf_iterator<char>(ext::ipstream::rdbuf())),
                               (std::istreambuf_iterator<char>()));
        }

        void Close()
        {
            ext::ipstream::close();
        }
    };

    OutputStream &StdIn()
    {
        if (_RunnableProcess::IsValid())
            return in_;
        if (_RunnableProcess::errorCallbackWithSelf_)
            _RunnableProcess::errorCallbackWithSelf_(*this, ERROR_INVALID_HANDLE,
                                                     std::runtime_error("Unknown process type"));
        throw std::runtime_error("Invalid proceses");
    }

    InputStream &StdOut()
    {
        if (_RunnableProcess::IsValid())
            return out_;
        if (_RunnableProcess::errorCallbackWithSelf_)
            _RunnableProcess::errorCallbackWithSelf_(*this, ERROR_INVALID_HANDLE,
                                                     std::runtime_error("Unknown process type"));
        throw std::runtime_error("Invalid proceses");
    }

    InputStream &StdErr()
    {
        if (_RunnableProcess::IsValid())
            return err_;
        if (_RunnableProcess::errorCallbackWithSelf_)
            _RunnableProcess::errorCallbackWithSelf_(*this, ERROR_INVALID_HANDLE,
                                                     std::runtime_error("Unknown process type"));
        throw std::runtime_error("Invalid proceses");
    }

  private:
    OutputStream in_;
    InputStream out_;
    InputStream err_;
    DWORD creationFlags_;
    BOOL inheritHandles_;
    LPVOID environmentBlock_;
    const typename Win32Ex::STARTUPINFOT<CharType>::Type *startupInfo_;

    //
    // RunnableProcessT Interface Implementations.
    //
  public:
    Waitable RunAsync() override
    {
        return RunAsync(_RunnableProcess::arguments_);
    }
};

template <typename StringType = StringT> class ElevatedProcessT : public RunnableProcessT<StringType>
{
  private:
    typedef RunnableProcessT<StringType> _RunnableProcess;

  public:
    typedef typename StringType::value_type CharType;

    ElevatedProcessT(const StringType &ExecutablePath, const Optional<const StringType &> &Arguments = None(),
                     const Optional<const StringType &> &CurrentDirectory = None(), DWORD CmdShow = SW_SHOWDEFAULT)
        : _RunnableProcess(ExecutablePath, Arguments, CurrentDirectory)
    {
        ZeroMemory(&executeInfo_, sizeof(executeInfo_));

        executeInfo_.cbSize = sizeof(executeInfo_);
        executeInfo_.fMask |= SEE_MASK_NOCLOSEPROCESS;
        executeInfo_.lpVerb = Details::Process::ConstValue<CharType>::RunAs();
        executeInfo_.lpFile = _RunnableProcess::executablePath_.c_str();

        if (_RunnableProcess::arguments_.IsSome())
            executeInfo_.lpParameters = _RunnableProcess::arguments_.Get();
        if (_RunnableProcess::currentDirectory_.IsSome())
            executeInfo_.lpDirectory = _RunnableProcess::currentDirectory_.Get();

        executeInfo_.nShow = CmdShow;
        if (CmdShow == SW_HIDE)
            executeInfo_.fMask |= SEE_MASK_NO_CONSOLE;

        _RunnableProcess::sessionId_ = WTSGetActiveConsoleSessionId();
    }

    ElevatedProcessT(typename SHELLEXECUTEINFOT<CharType>::Type &ExecuteInfo)
        : _RunnableProcess(ExecuteInfo.lpFile, ExecuteInfo.lpParameters, ExecuteInfo.lpDirectory)
    {
        executeInfo_ = ExecuteInfo;
        _RunnableProcess::sessionId_ = WTSGetActiveConsoleSessionId();
    }

    bool Run(const typename SHELLEXECUTEINFOT<CharType>::Type &ExecuteInfo)
    {
        return RunAsync(ExecuteInfo).Wait();
    }

    bool Run(const Optional<const StringType &> &Arguments = None(),
             const Optional<const StringType &> &CurrentDirectory = None(), const Optional<DWORD> &CmdShow = None())
    {
        if (Arguments.IsSome())
            _RunnableProcess::arguments_ = Arguments;

        return RunAsync(_RunnableProcess::arguments_, CurrentDirectory, CmdShow).Wait();
    }

    Waitable RunAsync(const Optional<const StringType &> &Arguments,
                      const Optional<const StringType &> &CurrentDirectory = None(),
                      const Optional<DWORD> &CmdShow = None())
    {
        if (!_RunnableProcess::Prepare())
            return Waitable(*this);

        executeInfo_.cbSize = sizeof(executeInfo_);
        executeInfo_.fMask |= SEE_MASK_NOCLOSEPROCESS;
        executeInfo_.lpVerb = Details::Process::ConstValue<CharType>::RunAs();
        executeInfo_.lpFile = _RunnableProcess::executablePath_.c_str();

        if (Arguments.IsSome())
        {
            _RunnableProcess::arguments_ = Arguments;
            executeInfo_.lpParameters = _RunnableProcess::arguments_.Get();
        }

        if (CurrentDirectory.IsSome())
        {
            _RunnableProcess::currentDirectory_ = CurrentDirectory;
            executeInfo_.lpDirectory = _RunnableProcess::currentDirectory_.Get();
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
        if (!_RunnableProcess::Prepare())
            return Waitable(*this);

        if (&ExecuteInfo != &executeInfo_)
        {
            _RunnableProcess::executablePath_ = ExecuteInfo.lpFile;
            _RunnableProcess::arguments_ = ExecuteInfo.lpParameters;
            _RunnableProcess::currentDirectory_ = ExecuteInfo.lpDirectory;
            executeInfo_ = ExecuteInfo;
        }

        if (!ShellExecuteExT<CharType>(&ExecuteInfo))
        {
            if (_RunnableProcess::errorCallbackWithSelf_)
                _RunnableProcess::errorCallbackWithSelf_(*this, GetLastError(),
                                                         std::runtime_error("Failed to ShellExecuteExT"));
            return Waitable(*this);
        }

        _RunnableProcess::processInfo_.dwProcessId = GetProcessId(ExecuteInfo.hProcess);
        _RunnableProcess::processInfo_.hProcess = ExecuteInfo.hProcess;

        _RunnableProcess::Finish();

        return Waitable(*this);
    }

  private:
    typename SHELLEXECUTEINFOT<CharType>::Type executeInfo_;

    //
    // RunnableProcessT Interface Implementations.
    //
  public:
    Waitable RunAsync() override
    {
        return RunAsync(executeInfo_);
    }
};

typedef ProcessT<String> Process;
typedef ProcessT<StringW> ProcessW;

typedef RunnableProcessT<String> RunnableProcess;
typedef RunnableProcessT<StringW> RunnableProcessW;

typedef ElevatedProcessT<String> ElevatedProcess;
typedef ElevatedProcessT<StringW> ElevatedProcessW;

typedef RunnableSessionProcessT<UserAccount, String> UserAccountProcess;
typedef RunnableSessionProcessT<UserAccount, StringW> UserAccountProcessW;
typedef RunnableSessionProcessT<UserAccount, StringT> UserAccountProcessT;

typedef RunnableSessionProcessT<SystemAccount, String> SystemAccountProcess;
typedef RunnableSessionProcessT<SystemAccount, StringW> SystemAccountProcessW;
typedef RunnableSessionProcessT<SystemAccount, StringT> SystemAccountProcessT;
} // namespace System

namespace ThisProcess
{
namespace Details
{
static DWORD mainThreadId = GetCurrentThreadId();
static HANDLE OpenMainThread();
static HANDLE mainThreadHandle = OpenMainThread();

WIN32EX_ALWAYS_INLINE VOID CloseMainThread()
{
    if (mainThreadHandle != NULL)
        CloseHandle(mainThreadHandle);
}

WIN32EX_ALWAYS_INLINE HANDLE OpenMainThread()
{
    HANDLE handle = OpenThread(MAXIMUM_ALLOWED, FALSE, GetCurrentThreadId());
    if (handle != NULL)
        atexit(CloseMainThread);
    return handle;
}
} // namespace Details

WIN32EX_ALWAYS_INLINE DWORD MainThreadId()
{
    return Details::mainThreadId;
}

WIN32EX_ALWAYS_INLINE HANDLE MainThreadHandle()
{
    return Details::mainThreadHandle;
}

WIN32EX_ALWAYS_INLINE DWORD Id()
{
    return GetCurrentProcessId();
}

WIN32EX_ALWAYS_INLINE HANDLE Handle()
{
    return GetCurrentProcess();
}

WIN32EX_ALWAYS_INLINE bool IsAdmin()
{
    return IsUserAdmin(GetCurrentProcessToken()) == TRUE;
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class StringType = StringT>
#else
template <class StringType>
#endif
WIN32EX_ALWAYS_INLINE StringType &ExecutablePathT()
{
    static StringType executablePath_(MAX_PATH, 0);
    size_t returnSize =
        GetModuleFileNameT<typename StringType::value_type>(NULL, &executablePath_[0], (DWORD)executablePath_.size());
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
            returnSize = GetModuleFileNameT<typename StringType::value_type>(NULL, &executablePath_[0],
                                                                             (DWORD)executablePath_.size());
            executablePath_.resize(returnSize);
        }
        if (GetLastError() == ERROR_SUCCESS)
            break;
    }
    return executablePath_;
}
WIN32EX_ALWAYS_INLINE String ExecutablePath()
{
    return ExecutablePathT<String>();
}
WIN32EX_ALWAYS_INLINE StringW ExecutablePathW()
{
    return ExecutablePathT<StringW>();
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class StringType = StringT>
#else
template <class StringType>
#endif
WIN32EX_ALWAYS_INLINE StringType &CommandLineT()
{
    static StringType cmdline_(MAX_PATH, 0);
    cmdline_ = GetCommandLineT<typename StringType::value_type>();
    return cmdline_;
}
WIN32EX_ALWAYS_INLINE String CommandLine()
{
    return CommandLineT<String>();
}
WIN32EX_ALWAYS_INLINE StringW CommandLineW()
{
    return CommandLineT<StringW>();
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class StringType = StringT>
#else
template <class StringType>
#endif
WIN32EX_ALWAYS_INLINE StringType CurrentDirectoryT()
{
    StringType cwd(MAX_PATH, 0);
    size_t length = Win32Ex::GetCurrentDirectoryT<typename StringType::value_type>((DWORD)cwd.size(), &cwd[0]);
    if (length > cwd.size())
    {
        cwd.resize(length);
        length = Win32Ex::GetCurrentDirectoryT<typename StringType::value_type>((DWORD)cwd.size(), &cwd[0]);
    }
    cwd.resize(length);
    return cwd;
}
WIN32EX_ALWAYS_INLINE String CurrentDirectory()
{
    return CurrentDirectoryT<String>();
}
WIN32EX_ALWAYS_INLINE StringW CurrentDirectoryW()
{
    return CurrentDirectoryT<StringW>();
}

#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
template <class StringType = StringT>
#else
template <class StringType>
#endif
WIN32EX_ALWAYS_INLINE System::ProcessT<StringType> ParentT()
{
    return System::ProcessT<StringType>(GetParentProcessId(ThisProcess::Id()));
}
WIN32EX_ALWAYS_INLINE System::Process Parent()
{
    return System::Process(GetParentProcessId(ThisProcess::Id()));
}
WIN32EX_ALWAYS_INLINE System::ProcessW ParentW()
{
    return System::ProcessW(GetParentProcessId(ThisProcess::Id()));
}
} // namespace ThisProcess
} // namespace Win32Ex

#undef _STD_NS_

#endif // _WIN32EX_SYSTEM_PROCESS_HPP_
