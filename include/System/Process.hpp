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

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "..\Internal\misc.hpp"
#include "..\Security\Token.h"
#include "process.h"
#include <atlconv.h>
#include <functional>

#if _MSC_VER < 1600
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
template <ProcessAccountType _Type> class Process
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
        //
        //  WithSelf Callback호출 시, Callback도 호출되도록 설정합니다.
        //
        enterCallbackWithSelf_ =
            _STD_NS_::bind(&Process<_Type>::enterCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1);
        exitCallbackWithSelf_ =
            _STD_NS_::bind(&Process<_Type>::exitCallbackWithSelfDefault_, this, _STD_NS_::placeholders::_1);
        errorCallbackWithSelf_ = _STD_NS_::bind(&Process<_Type>::errorCallbackWithSelfDefault_, this,
                                                _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2);
    }

  public:
    Process(ProcessId ProcessId) : creationFlags_(0), exitDetectionThread_(NULL)
    {
        ProcessIdToSessionId(ProcessId, &sessionId_);

        processInfo_.dwProcessId = ProcessId;

        HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessId);
        if (hProcess)
        {
            CHAR processName[4096];
            DWORD processNameLength = _countof(processName);

            if (QueryFullProcessImageNameA(hProcess, 0, processName, &processNameLength))
            {
                name_.assign(processName, processNameLength);
            }

            processInfo_.hProcess = hProcess;
        }
        else
        {
            processInfo_.hProcess = NULL;
        }

        processInfo_.dwThreadId = 0;
        processInfo_.hThread = NULL;
        hThreadStopEvent_ = NULL;

        initCallbacks_();
    }

    Process(const std::string &ProcessPathName, const char *Arguments = NULL, const char *CurrentDirectory = NULL)
        : name_(ProcessPathName), creationFlags_(0), exitDetectionThread_(NULL)
    {
        if (Arguments)
            arguments_ = Arguments;
        if (CurrentDirectory)
            currentDirectory_ = CurrentDirectory;
        ZeroMemory(&processInfo_, sizeof(processInfo_));
        ProcessIdToSessionId(GetCurrentProcessId(), &sessionId_);
        hThreadStopEvent_ = NULL;

        initCallbacks_();
    }

    Process(DWORD SessionId, const char *ProcessPathName, const char *Arguments = NULL,
            const char *CurrentDirectory = NULL, DWORD CreationFlags = 0L)
        : sessionId_(SessionId), name_(ProcessPathName), creationFlags_(CreationFlags), exitDetectionThread_(NULL)
    {
        if (Arguments)
            arguments_ = Arguments;
        if (CurrentDirectory)
            currentDirectory_ = CurrentDirectory;

        ZeroMemory(&processInfo_, sizeof(processInfo_));
        hThreadStopEvent_ = NULL;

        initCallbacks_();
    }

    ~Process()
    {
        //
        //  스레드 종료 이벤트를 시그널시킵니다.
        //

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

        //
        //  스레드, 프로세스 핸들을 정리합니다.
        // 프로세스 종료 감지 스레드가 생성되지 않을수도 있기때문에 소멸자에서 정리했습니다.
        //

        handle = InterlockedExchangePointer(&processInfo_.hProcess, NULL);
        if (handle)
            CloseHandle(handle);

        handle = InterlockedExchangePointer(&processInfo_.hThread, NULL);
        if (handle)
            CloseHandle(handle);
    }

    bool Wait(unsigned long Timeout = INFINITE)
    {
        if (processInfo_.hProcess == NULL)
            return false;

        if (WaitForSingleObject(processInfo_.hProcess, (DWORD)Timeout) == WAIT_OBJECT_0)
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

    bool Run(const char *Arguments = NULL, const char *CurrentDirectory = NULL, DWORD CreationFlags = 0L)
    {
        if (IsRunning())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, std::exception("Already running"));
            return false;
        }

        return RunAsync(Arguments, CurrentDirectory, CreationFlags).Wait();
    }

    bool IsRunning()
    {
        return (processInfo_.hProcess != NULL);
    }

    Process &RunAsync(const char *Arguments = NULL, const char *CurrentDirectory = NULL, DWORD CreationFlags = 0L)
    {
        if (IsRunning())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, std::exception("Already running"));
            return *this;
        }

        if (name_.empty())
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, std::exception("Process image file name is not specified"));
            return *this;
        }

        std::basic_string<TCHAR> command;
        std::basic_string<TCHAR> currentDir;
        if (Arguments)
        {
            arguments_ = Arguments;
        }

        if (CurrentDirectory)
        {
            currentDirectory_ = CurrentDirectory;
        }

        if (CreationFlags)
        {
            creationFlags_ = CreationFlags;
        }
#if _UNICODE
        currentDir = !currentDirectory_;
        command = (arguments_.empty()) ? !name_ : !name_ + TEXT(" ") + !arguments_;
#else
        currentDir = currentDirectory_;
        command = (arguments_.empty()) ? name_ : name_ + TEXT(" ") + arguments_;
#endif
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        if (_Type == SystemAccount)
        {
            if (!CreateSystemAccountProcess(sessionId_, NULL, &command[0], NULL, NULL, FALSE, creationFlags_, NULL,
                                            currentDir.empty() ? NULL : currentDir.c_str(), &si, &processInfo_))
            {
                if (errorCallbackWithSelf_)
                    errorCallbackWithSelf_(*this, std::exception("Failed to CreateSystemAccountProcess"));
                return *this;
            }
        }
        else if (_Type == UserAccount)
        {
            if (!CreateUserAccountProcess(sessionId_, NULL, &command[0], NULL, NULL, FALSE, creationFlags_, NULL,
                                          currentDir.empty() ? NULL : currentDir.c_str(), &si, &processInfo_))
            {
                if (errorCallbackWithSelf_)
                    errorCallbackWithSelf_(*this, std::exception("Failed to CreateUserAccountProcess"));
                return *this;
            }
        }
        else
        {
            if (errorCallbackWithSelf_)
                errorCallbackWithSelf_(*this, std::exception("Unknown process type"));
            return *this;
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
                errorCallbackWithSelf_(*this, std::exception("Failed to CreateEvent"));
            return *this;
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
        return *this;
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

    const std::string &GetExecutablePath() const
    {
        return name_;
    }

    operator const std::string() const
    {
        std::stringstream process_info;
        process_info << name_ << "(" << processInfo_.dwProcessId << ")";
        return process_info.str();
    }

    DWORD GetMainThreadId() const
    {
        return processInfo_.dwThreadId;
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
                self->errorCallbackWithSelf_(*self, std::exception("Failed to WaitForSingleObject"));
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
    PROCESS_INFORMATION processInfo_;

    HANDLE hThreadStopEvent_;
    HANDLE exitDetectionThread_;

    ProcessEnterCallback enterCallback_;
    ProcessExitCallback exitCallback_;
    ProcessErrorCallback errorCallback_;

    ProcessEnterCallbackWithSelf enterCallbackWithSelf_;
    ProcessExitCallbackWithSelf exitCallbackWithSelf_;
    ProcessErrorCallbackWithSelf errorCallbackWithSelf_;
};
} // namespace Detail

typedef Detail::Process<SystemAccount> SystemAccountProcess;
typedef Detail::Process<UserAccount> UserAccountProcess;

} // namespace System

//
//  현재 프로세스 관련 정의.
//

namespace ThisProcess
{
static std::string &GetExecutablePath()
{
    static std::string processName_;
    USES_CONVERSION;

    if (!processName_.empty())
        return processName_;

    //
    //  첫번째 인자가 프로그램 전체 경로라는 조건은 보장되지 않으므로 문제가 된다면 코드를 삭제하십시오.
    //

    if (__argv)
    {
        processName_.assign(__argv[0]);
        if (!processName_.empty())
            return processName_;
    }

    if (__wargv)
    {
        processName_.assign((W2A(std::wstring(__wargv[0]).c_str())));
        if (!processName_.empty())
            return processName_;
    }

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

static bool IsSystemAccount()
{
    return IsUserAdmin(GetCurrentProcessToken()) == TRUE;
}
} // namespace ThisProcess
} // namespace Win32Ex

#undef _STD_NS_
