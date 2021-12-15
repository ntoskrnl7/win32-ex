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

#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")

#include "../Internal/misc.hpp"
#include "../Security/Token.h"
#include "Process.h"
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
    static std::string processName_(MAX_PATH, '\0');
    size_t returnSize = GetModuleFileNameA(NULL, &processName_[0], (DWORD)processName_.size());
    for (;;)
    {
        if (returnSize < processName_.size())
        {
            processName_.resize(returnSize);
            break;
        }
        else
        {
            processName_.resize(returnSize + MAX_PATH);
            returnSize = GetModuleFileNameA(NULL, &processName_[0], (DWORD)processName_.size());
            processName_.resize(returnSize);
        }
        if (GetLastError() == ERROR_SUCCESS)
        {
            break;
        }
    }
    return processName_;
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

static bool IsSystemAccount()
{
    return IsUserAdmin(GetCurrentProcessToken()) == TRUE;
}
} // namespace ThisProcess
namespace System
{
enum ProcessAccountType
{
    SystemAccount,
    UserAccount,
    ElevatedUserAccount
};

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

namespace Detail
{
template <ProcessAccountType _Type> class Process : public WaitableObject
{
    typedef std::function<void(Process<_Type> &)> ProcessEnterCallbackWithSelf;
    typedef std::function<void(Process<_Type> &)> ProcessExitCallbackWithSelf;
    typedef std::function<void(Process<_Type> &, DWORD, const std::exception &)> ProcessErrorCallbackWithSelf;

    typedef std::function<void()> ProcessEnterCallback;
    typedef std::function<void()> ProcessExitCallback;
    typedef std::function<void(DWORD, const std::exception &)> ProcessErrorCallback;

  private:
    void initCallbacks_()
    {
        enterCallbackWithSelf_ =
            _STD_NS_::bind(&Process<_Type>::enterCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1);
        exitCallbackWithSelf_ =
            _STD_NS_::bind(&Process<_Type>::exitCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1);
        errorCallbackWithSelf_ =
            _STD_NS_::bind(&Process<_Type>::errorCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1,
                           _STD_NS_::placeholders::_2, _STD_NS_::placeholders::_3);
    }

    VOID Attach_(HANDLE ProcessHandle)
    {
        DWORD processId = GetProcessId(ProcessHandle);
        if (processId != 0)
            ProcessIdToSessionId(processId, &sessionId_);

        CHAR processName[4096];
        DWORD processNameLength = sizeof(processName) / sizeof(CHAR);
        if (QueryFullProcessImageNameA(ProcessHandle, 0, processName, &processNameLength))
            name_.assign(processName, processNameLength);

        processInfo_.hProcess = ProcessHandle;
        processInfo_.dwProcessId = processId;
        processInfo_.dwThreadId = 0;
        processInfo_.hThread = NULL;
    }

    VOID Detach_()
    {
        HANDLE handle = InterlockedExchangePointer(&hThreadStopEvent_, NULL);
        if (handle)
        {
            SetEvent(handle);
            CloseHandle(handle);
        }

        if (exitDetectionThread_)
            WaitForSingleObject(exitDetectionThread_, INFINITE);

        handle = InterlockedExchangePointer(&processInfo_.hProcess, NULL);
        if (handle)
            CloseHandle(handle);

        handle = InterlockedExchangePointer(&processInfo_.hThread, NULL);
        if (handle)
            CloseHandle(handle);
    }

  public:
    BOOL Attach(ProcessHandle ProcessHandle)
    {
        if (processInfo_.hProcess || hThreadStopEvent_ || exitDetectionThread_)
        {
            SetLastError(ERROR_ALREADY_INITIALIZED);
            return FALSE;
        }
        Attach_(ProcessHandle.value);
        return TRUE;
    }

    Process(ProcessHandle ProcessHandle) : exitDetectionThread_(NULL), hThreadStopEvent_(NULL), creationFlags_(0)
    {
        ZeroMemory(&processInfo_, sizeof(processInfo_));
        initCallbacks_();
        Attach_(ProcessHandle.value);
    }

    Process(ProcessId ProcessId) : exitDetectionThread_(NULL), hThreadStopEvent_(NULL), creationFlags_(0)
    {
        ZeroMemory(&processInfo_, sizeof(processInfo_));
        initCallbacks_();
        HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessId);
        if (hProcess)
            Attach_(hProcess);
    }

    Process(const std::string &ProcessPathName, const std::string &Arguments = std::string(),
            const std::string &CurrentDirectory = std::string(), DWORD CreationFlags = 0L,
            const LPSTARTUPINFOA StartupInfo = NULL, BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL)
        : exitDetectionThread_(NULL), hThreadStopEvent_(NULL), name_(ProcessPathName), arguments_(Arguments),
          currentDirectory_(CurrentDirectory), creationFlags_(CreationFlags), startupInfo_(StartupInfo),
          inheritHandles_(InheritHandles), environmentBlock_(EnvironmentBlock)
    {
        ZeroMemory(&processInfo_, sizeof(processInfo_));
        initCallbacks_();
        ProcessIdToSessionId(GetCurrentProcessId(), &sessionId_);
    }

    Process(DWORD SessionId, const std::string &ProcessPathName, const std::string &Arguments = std::string(),
            const std::string &CurrentDirectory = std::string(), DWORD CreationFlags = 0L,
            const LPSTARTUPINFOA StartupInfo = NULL, BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL)
        : exitDetectionThread_(NULL), hThreadStopEvent_(NULL), name_(ProcessPathName), arguments_(Arguments),
          currentDirectory_(CurrentDirectory), creationFlags_(CreationFlags), startupInfo_(StartupInfo),
          inheritHandles_(InheritHandles), environmentBlock_(EnvironmentBlock), sessionId_(SessionId)
    {
        ZeroMemory(&processInfo_, sizeof(processInfo_));
        initCallbacks_();
    }

    ~Process()
    {
        Detach_();
    }

    bool Run(PCSTR Arguments = NULL, PCSTR CurrentDirectory = NULL, DWORD CreationFlags = 0L,
             const LPSTARTUPINFOA StartupInfo = NULL, BOOL InheritHandles = FALSE, LPVOID EnvironmentBlock = NULL)
    {
        if (IsRunning())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, GetLastError(), std::runtime_error("Already running"));
            return false;
        }

        return RunAsync(Arguments, CurrentDirectory, CreationFlags, StartupInfo, InheritHandles, EnvironmentBlock)
            .Wait();
    }

    bool IsRunning()
    {
        return (processInfo_.hProcess != NULL);
    }

    Waitable RunAsync(PCSTR Arguments = NULL, PCSTR CurrentDirectory = NULL, DWORD CreationFlags = 0L,
                      const LPSTARTUPINFOA StartupInfo = NULL, BOOL InheritHandles = FALSE,
                      LPVOID EnvironmentBlock = NULL)
    {
        if (IsRunning())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, ERROR_BUSY, std::runtime_error("Already running"));
            return Waitable(*this);
        }

        if (name_.empty())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, ERROR_INVALID_NAME,
                                       std::runtime_error("Process image file name is not specified"));
            return Waitable(*this);
        }

        if (Arguments)
            arguments_ = Arguments;

        if (CurrentDirectory)
            currentDirectory_ = CurrentDirectory;

        if (CreationFlags)
            creationFlags_ = CreationFlags;

        if (CreationFlags)
            creationFlags_ = CreationFlags;

        if (InheritHandles)
            inheritHandles_ = InheritHandles;

        if (EnvironmentBlock)
            environmentBlock_ = EnvironmentBlock;

        if (_Type == ElevatedUserAccount)
        {
            if (ThisProcess::IsSystemAccount())
            {
                if (errorCallbackWithSelf_)
                    errorCallbackWithSelf_(*this, ERROR_NOT_SUPPORTED,
                                           std::runtime_error("It can only be called by user account process."));
                return Waitable(*this);
            }
            else
            {
#if _UNICODE
                Win32Ex::TString name = !name_;
                Win32Ex::TString params = !arguments_;
                Win32Ex::TString cwd = !currentDirectory_;
#else
                Win32Ex::TString name = name_;
                Win32Ex::TString params = arguments_;
                Win32Ex::TString cwd = currentDirectory_;
#endif
                SHELLEXECUTEINFO sei = {
                    0,
                };
                sei.fMask |= SEE_MASK_NOCLOSEPROCESS;
                sei.cbSize = sizeof(SHELLEXECUTEINFO);
                sei.lpVerb = TEXT("runas");
                sei.lpFile = name.c_str();

                if (!params.empty())
                    sei.lpParameters = params.c_str();

                if (!cwd.empty())
                    sei.lpDirectory = cwd.c_str();

                if (creationFlags_ & CREATE_NO_WINDOW)
                {
                    sei.fMask |= SEE_MASK_FLAG_NO_UI;
                }

                sei.nShow = StartupInfo ? StartupInfo->wShowWindow : SW_SHOWDEFAULT;

                if (!ShellExecuteEx(&sei))
                {
                    if (errorCallbackWithSelf_)
                        errorCallbackWithSelf_(*this, GetLastError(), std::runtime_error("Failed to ShellExecuteEx"));
                    return Waitable(*this);
                }

                processInfo_.dwProcessId = GetProcessId(sei.hProcess);
                processInfo_.hProcess = sei.hProcess;
            }
        }
        else
        {
            Win32Ex::TString command;
            Win32Ex::TString currentDir;
            STARTUPINFOEX si;
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
                si.StartupInfo.cb = sizeof(STARTUPINFO);
            }
#if _UNICODE
            std::wstring desktop;
            std::wstring title;

            using namespace Convert::String;
            currentDir = !currentDirectory_;
            command = (arguments_.empty()) ? !name_ : !(name_ + " " + arguments_);
            if (startupInfo_)
            {
                if (startupInfo_->lpDesktop)
                {
                    desktop = !std::string(startupInfo_->lpDesktop);
                    si.StartupInfo.lpDesktop = &desktop[0];
                }
                if (startupInfo_->lpTitle)
                {
                    title = !std::string(startupInfo_->lpTitle);
                    si.StartupInfo.lpTitle = &title[0];
                }
            }
#else
            currentDir = currentDirectory_;
            command = (arguments_.empty()) ? name_ : name_ + TEXT(" ") + arguments_;
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
                si.StartupInfo.cb = sizeof(STARTUPINFO);
            }
#endif
            if (_Type == SystemAccount)
            {
                if (!CreateSystemAccountProcess(
                        sessionId_, NULL, &command[0], NULL, NULL, InheritHandles, creationFlags_, environmentBlock_,
                        currentDir.empty() ? NULL : currentDir.c_str(), &si.StartupInfo, &processInfo_))
                {
                    if (errorCallbackWithSelf_)
                        errorCallbackWithSelf_(*this, GetLastError(),
                                               std::runtime_error("Failed to CreateSystemAccountProcess"));
                    return Waitable(*this);
                }
            }
            else if (_Type == UserAccount)
            {
                if (!CreateUserAccountProcess(sessionId_, NULL, &command[0], NULL, NULL, InheritHandles, creationFlags_,
                                              environmentBlock_, currentDir.empty() ? NULL : currentDir.c_str(),
                                              &si.StartupInfo, &processInfo_))
                {
                    if (errorCallbackWithSelf_)
                        errorCallbackWithSelf_(*this, GetLastError(),
                                               std::runtime_error("Failed to CreateUserAccountProcess"));
                    return Waitable(*this);
                }
            }
            else
            {
                if (errorCallbackWithSelf_)
                    errorCallbackWithSelf_(*this, ERROR_CLASS_DOES_NOT_EXIST,
                                           std::runtime_error("Unknown process type"));
                return Waitable(*this);
            }
        }

        if (enterCallbackWithSelf_)
            enterCallbackWithSelf_(*this);

        HANDLE handle = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (InterlockedCompareExchangePointer(&hThreadStopEvent_, handle, NULL))
        {
            if (handle)
                CloseHandle(handle);
        }
        if (hThreadStopEvent_ == NULL)
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, GetLastError(), std::runtime_error("Failed to CreateEvent"));
            return Waitable(*this);
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

        exitDetectionThread_ = CreateThread(NULL, 0, &Process<_Type>::ExitDetectionThreadProc_, this, 0, NULL);
        return Waitable(*this);
    }

    bool Exit()
    {
        if (IsRunning())
            return TerminateProcess(processInfo_.hProcess, ERROR_PROCESS_ABORTED) == TRUE;
        return false;
    }

    Process &OnEnterEx(ProcessEnterCallbackWithSelf Callback)
    {
        enterCallbackWithSelf_ = Callback;
        return *this;
    }

    Process &OnExitEx(ProcessExitCallbackWithSelf Callback)
    {
        exitCallbackWithSelf_ = Callback;
        return *this;
    }

    Process &OnErrorEx(ProcessErrorCallbackWithSelf Callback)
    {
        errorCallbackWithSelf_ = Callback;
        return *this;
    }

    Process &OnEnter(ProcessEnterCallback Callback)
    {
        enterCallback_ = Callback;
        return *this;
    }

    Process &OnExit(ProcessExitCallback Callback)
    {
        exitCallback_ = Callback;
        return *this;
    }

    Process &OnError(ProcessErrorCallback Callback)
    {
        errorCallback_ = Callback;
        return *this;
    }

    operator const std::string() const
    {
        std::stringstream process_info;
        process_info << name_ << "(" << processInfo_.dwProcessId << ")";
        return process_info.str();
    }

    const std::string &GetExecutablePath() const
    {
        return name_;
    }

    DWORD GetId() const
    {
        return processInfo_.dwProcessId;
    }

    HANDLE GetHandle() const
    {
        return processInfo_.hProcess;
    }

    DWORD GetMainThreadId() const
    {
        return processInfo_.dwThreadId;
    }

    HANDLE GetMainThreadHandle() const
    {
        return processInfo_.hThread;
    }

    bool IsSystemAccount() const
    {
        bool bRet = (_Type == SystemAccount);
        HANDLE hToken;
        if (!OpenProcessToken(processInfo_.hProcess, TOKEN_QUERY | TOKEN_IMPERSONATE, &hToken))
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

  private:
    void enterCallbackWithSelfDefault_(Process<_Type> &)
    {
        if (enterCallback_)
            enterCallback_();
    };

    void exitCallbackWithSelfDefault_(Process<_Type> &)
    {
        if (exitCallback_)
            exitCallback_();
    };

    void errorCallbackWithSelfDefault_(Process<_Type> &, DWORD LastError, const std::exception &e)
    {
        if (errorCallback_)
            errorCallback_(LastError, e);
    };

    static DWORD WINAPI ExitDetectionThreadProc_(PVOID lpThreadParameter)
    {
        Process *self = (Process *)lpThreadParameter;
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

  private:
    std::string name_;
    std::string arguments_;
    std::string currentDirectory_;
    DWORD sessionId_;
    DWORD creationFlags_;

    BOOL inheritHandles_;
    LPVOID environmentBlock_;

    PROCESS_INFORMATION processInfo_;
    LPSTARTUPINFOA startupInfo_;

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
    bool IsWaitable()
    {
        return processInfo_.hProcess && (processInfo_.dwProcessId != GetCurrentProcessId());
    }

    bool Wait(Duration Timeout)
    {
        if (WaitForSingleObject(processInfo_.hProcess, Timeout) == WAIT_OBJECT_0)
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
}; // namespace Detail
} // namespace Detail

typedef Detail::Process<SystemAccount> SystemAccountProcess;
typedef Detail::Process<UserAccount> UserAccountProcess;
typedef Detail::Process<ElevatedUserAccount> ElevatedUserAccountProcess;

} // namespace System
} // namespace Win32Ex

#undef _STD_NS_

#endif // _WIN32EX_SYSTEM_PROCESS_HPP_