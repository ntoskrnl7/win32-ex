﻿/*++

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

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <Windows.h>

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
namespace System
{
enum ProcessAccountType
{
    SystemAccount,
    UserAccount
};

typedef DWORD ProcessId;

namespace Detail
{
template <ProcessAccountType _Type> class Process : public WaitableObject
{
    typedef std::function<void(Process<_Type> &)> ProcessEnterCallbackWithSelf;
    typedef std::function<void(Process<_Type> &)> ProcessExitCallbackWithSelf;
    typedef std::function<void(Process<_Type> &, const std::exception &)> ProcessErrorCallbackWithSelf;

    typedef std::function<void()> ProcessEnterCallback;
    typedef std::function<void()> ProcessExitCallback;
    typedef std::function<void(const std::exception &)> ProcessErrorCallback;

  private:
    void initCallbacks_()
    {
        enterCallbackWithSelf_ =
            _STD_NS_::bind(&Process<_Type>::enterCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1);
        exitCallbackWithSelf_ =
            _STD_NS_::bind(&Process<_Type>::exitCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1);
        errorCallbackWithSelf_ = _STD_NS_::bind(&Process<_Type>::errorCallbackWithSelfDefault_, this,
                                                _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2);
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
    BOOL Attach(HANDLE ProcessHandle)
    {
        if (processInfo_.hProcess || hThreadStopEvent_ || exitDetectionThread_)
        {
            SetLastError(ERROR_ALREADY_INITIALIZED);
            return FALSE;
        }
        Attach_(ProcessHandle);
        return TRUE;
    }

    // Process(HANDLE ProcessHandle) : exitDetectionThread_(NULL), hThreadStopEvent_(NULL), creationFlags_(0)
    // {
    //     ZeroMemory(&processInfo_, sizeof(processInfo_));
    //     initCallbacks_();
    //     Attach_(ProcessHandle);
    // }

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
                errorCallbackWithSelf_(*this, std::runtime_error("Already running"));
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
                errorCallbackWithSelf_(*this, std::runtime_error("Already running"));
            return Waitable(*this);
        }

        if (name_.empty())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, std::runtime_error("Process image file name is not specified"));
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

        std::basic_string<TCHAR> command;
        std::basic_string<TCHAR> currentDir;
        STARTUPINFOEX si;
        if (startupInfo_)
        {
            CopyMemory(&si, startupInfo_, std::min((DWORD)sizeof(si), startupInfo_->cb));
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
            CopyMemory(&si, startupInfo_, std::min((DWORD)sizeof(si), startupInfo_->cb));
        }
        else
        {
            ZeroMemory(&si, sizeof(si));
            si.StartupInfo.cb = sizeof(STARTUPINFO);
        }
#endif
        if (_Type == SystemAccount)
        {
            if (!CreateSystemAccountProcess(sessionId_, NULL, &command[0], NULL, NULL, InheritHandles, creationFlags_,
                                            environmentBlock_, currentDir.empty() ? NULL : currentDir.c_str(),
                                            &si.StartupInfo, &processInfo_))
            {
                if (errorCallbackWithSelf_)
                    errorCallbackWithSelf_(*this, std::runtime_error("Failed to CreateSystemAccountProcess"));
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
                    errorCallbackWithSelf_(*this, std::runtime_error("Failed to CreateUserAccountProcess"));
                return Waitable(*this);
            }
        }
        else
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, std::runtime_error("Unknown process type"));
            return Waitable(*this);
        }

        //
        //  프로세스가 실행되었음을 알립니다.
        //

        if (enterCallbackWithSelf_)
            enterCallbackWithSelf_(*this);

        //
        // 스레드 종료 이벤트를 생성합니다.
        //

        HANDLE handle = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (InterlockedCompareExchangePointer(&hThreadStopEvent_, handle, NULL))
        {
            // 기존에 이벤트 핸들이 존재한다면 새로 생성한 핸들을 닫습니다.
            if (handle)
                CloseHandle(handle);
        }
        if (hThreadStopEvent_ == NULL)
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, std::runtime_error("Failed to CreateEvent"));
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
                //  exitDetectionThread_에서 RunAsync가 호출된것이 아니라면,  exitDetectionThread_ 스레드 종료 이벤트를
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

    void errorCallbackWithSelfDefault_(Process<_Type> &, const std::exception &e)
    {
        if (errorCallback_)
            errorCallback_(e);
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
                self->errorCallbackWithSelf_(*self, std::runtime_error("Failed to WaitForSingleObject"));
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
  private:
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

} // namespace System

namespace ThisProcess
{
static std::string &GetExecutablePath()
{
    static std::string processName_;
    DWORD fileNameSize = MAX_PATH;
    PSTR fileName = new CHAR[fileNameSize + 1];
    if (fileName == NULL)
        return processName_;

    DWORD returnSize = GetModuleFileNameA(NULL, fileName, fileNameSize);
    if (returnSize + 1 > fileNameSize)
    {
        delete[] fileName;

        fileNameSize = returnSize + 1;
        fileName = new CHAR[fileNameSize + 1];

        if (fileName == NULL)
            return processName_;

        returnSize = GetModuleFileNameA(NULL, fileName, fileNameSize);
        if (returnSize == 0)
        {
            delete[] fileName;
            return processName_;
        }
    }

    processName_.assign(fileName, returnSize);
    delete[] fileName;

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

static bool IsSystemAccount()
{
    return IsUserAdmin(GetCurrentProcessToken()) == TRUE;
}
} // namespace ThisProcess
} // namespace Win32Ex

#undef _STD_NS_
