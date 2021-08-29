#include <System\Process.hpp>
#include <gtest\gtest.h>
#include <stdio.h>
#include <tchar.h>

TEST(ProcessTest, ThisProcess)
{
    std::string path = Win32Ex::ThisProcess::GetExecutablePath();
    EXPECT_EQ(path, Win32Ex::ThisProcess::GetExecutablePath());
}

TEST(ProcessTest, RunInvalidProcess)
{
    Win32Ex::System::UserAccountProcess process("!@#$test");
    EXPECT_FALSE(process.Run());
}

#if !defined(__cpp_lambdas)
void onEnter(Win32Ex::System::UserAccountProcess *process)
{
    Sleep(500);
    process->Exit();
}
void onExit(bool *terminated)
{
    *terminated = true;
}
void onError(bool *terminated, const std::exception &)
{
    *terminated = true;
}
#endif

#if _MSC_VER < 1600
#define _STD_NS_ std::tr1
#else
#define _STD_NS_ std
#endif

#if !defined(__cpp_lambdas)
void onEnterSystemProcess(Win32Ex::System::SystemAccountProcess *process, bool *isSystemAccount)
{
    Sleep(500);
    *isSystemAccount = process->IsSystemAccount();
    process->Exit();
}
#endif

TEST(ProcessTest, RunSystemAccountProcess)
{
    if (Win32Ex::ThisProcess::IsSystemAccount())
    {
        Win32Ex::System::SystemAccountProcess process(WTSGetActiveConsoleSessionId(), "CMD /C WHOAMI /ALL");
        EXPECT_TRUE(process.Run());

        PSTR cmd = new CHAR[1024];
        if (cmd)
        {
            if (ExpandEnvironmentStringsA("%WINDIR%\\notepad.exe", cmd, 1024))
            {
                Win32Ex::System::SystemAccountProcess process(cmd);
                bool terminated = false;
#if defined(__cpp_lambdas)
                process
                    .OnEnter([&process]() {
                        Sleep(500);
                        EXPECT_TRUE(process.IsSystemAccount());
                        process.Exit();
                    })
                    .OnExit([&terminated]() { terminated = true; })
                    .OnError([&terminated](const std::exception &) { terminated = true; });
#else
                bool isSystemAccount;
                process.OnEnter(_STD_NS_::bind(onEnterSystemProcess, &process, &isSystemAccount))
                    .OnExit(_STD_NS_::bind(onExit, &terminated))
                    .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1));
#endif
                EXPECT_FALSE(terminated);
                EXPECT_TRUE(process.Run());
#if !defined(__cpp_lambdas)
                EXPECT_TRUE(isSystemAccount);
#endif
                EXPECT_TRUE(terminated);
            }
            delete cmd;
        }
    }
}

TEST(ProcessTest, RunUserAccountProcess)
{
    Win32Ex::System::UserAccountProcess process(WTSGetActiveConsoleSessionId(), "CMD /C WHOAMI /ALL");
    EXPECT_TRUE(process.Run());

    PSTR cmd = new CHAR[1024];
    if (cmd)
    {
        if (ExpandEnvironmentStringsA("%WINDIR%\\notepad.exe", cmd, 1024))
        {
            Win32Ex::System::UserAccountProcess process(cmd);
            bool terminated = false;
#if defined(__cpp_lambdas)
            process
                .OnEnter([&process]() {
                    Sleep(500);
                    EXPECT_FALSE(process.IsSystemAccount());
                    process.Exit();
                })
                .OnExit([&terminated]() { terminated = true; })
                .OnError([&terminated](const std::exception &) { terminated = true; });
#else
            process.OnEnter(_STD_NS_::bind(onEnter, &process))
                .OnExit(_STD_NS_::bind(onExit, &terminated))
                .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1));
#endif

            EXPECT_FALSE(terminated);
            EXPECT_TRUE(process.Run());
            EXPECT_TRUE(terminated);
        }
        delete cmd;
    }
}

#if !defined(__cpp_lambdas)
void onExitCV(PCONDITION_VARIABLE cv, bool *terminated)
{
    WakeConditionVariable(cv);
    *terminated = true;
}
void onErrorCV(PCONDITION_VARIABLE cv, bool *terminated, const std::exception &)
{
    WakeConditionVariable(cv);
    *terminated = true;
}
#endif

TEST(ProcessTest, RunAsyncUserAccountProcess)
{
    PSTR cmd = new CHAR[1024];
    if (cmd)
    {
        if (ExpandEnvironmentStringsA("%WINDIR%\\notepad.exe", cmd, 1024))
        {
            CRITICAL_SECTION cs;
            CONDITION_VARIABLE cv;
            InitializeConditionVariable(&cv);
            InitializeCriticalSection(&cs);

            Win32Ex::System::UserAccountProcess process(cmd);
            bool terminated = false;
#if defined(__cpp_lambdas)
            process
                .OnEnter([&process]() {
                    Sleep(500);
                    process.Exit();
                })
                .OnExit([&cv, &terminated]() {
                    terminated = true;
                    WakeConditionVariable(&cv);
                })
                .OnError([&cv, &terminated](const std::exception &) {
                    terminated = true;
                    WakeConditionVariable(&cv);
                });
#else
            process.OnEnter(_STD_NS_::bind(onEnter, &process))
                .OnExit(_STD_NS_::bind(onExitCV, &cv, &terminated))
                .OnError(_STD_NS_::bind(onErrorCV, &cv, &terminated, _STD_NS_::placeholders::_1));
#endif
            EXPECT_FALSE(terminated);
            process.RunAsync();
            SleepConditionVariableCS(&cv, &cs, INFINITE);
            EXPECT_TRUE(terminated);
        }
        delete cmd;
    }
}

TEST(ProcessTest, UserAccountProcessClassTest)
{
    PVOID OldValue = NULL;
    Wow64DisableWow64FsRedirection(&OldValue);

    PWTS_SESSION_INFO sessionInfo = NULL;
    DWORD count = 0;
    if (WTSEnumerateSessions(WTS_CURRENT_SERVER, 0, 1, &sessionInfo, &count))
    {
        BOOL isLocalSystem = IsLocalSystemToken(GetCurrentProcessToken());
        BOOL ret = FALSE;
        for (DWORD i = 0; i < count; ++i)
        {
            if (sessionInfo[i].State == WTSListen)
                continue;
#if _MSC_VER > 1600
            printf("UserAccountProcess (SessionId: %d, pWinStationName: %Ts, State: %d)\n",
#else
#ifdef _UNICODE
            wprintf(L"UserAccountProcess (SessionId: %d, pWinStationName: %s, State: %d)\n",
#else
            printf("UserAccountProcess (SessionId: %d, pWinStationName: %s, State: %d)\n",
#endif
#endif
                   sessionInfo[i].SessionId, sessionInfo[i].pWinStationName, sessionInfo[i].State);

            Win32Ex::System::UserAccountProcess process(sessionInfo[i].SessionId, "CMD /C WHOAMI /ALL");
            ret = process.Run();
            if (!ret)
            {
                printf("Failed to UserAccountProcess::Run(%d) : %d\n", sessionInfo[i].SessionId, GetLastError());
            }

            if (isLocalSystem || WTSGetActiveConsoleSessionId() == sessionInfo[i].SessionId)
            {
                EXPECT_EQ(ret, TRUE);
            }
        }

        WTSFreeMemory(sessionInfo);
    }
    Wow64RevertWow64FsRedirection(OldValue);
}

TEST(ProcessTest, SystemAccountProcessClassTest)
{
    if (Win32Ex::ThisProcess::IsSystemAccount())
    {
        PVOID OldValue = NULL;
        Wow64DisableWow64FsRedirection(&OldValue);

        PWTS_SESSION_INFO sessionInfo = NULL;
        DWORD count = 0;
        if (WTSEnumerateSessions(WTS_CURRENT_SERVER, 0, 1, &sessionInfo, &count))
        {
            BOOL ret = FALSE;
            for (DWORD i = 0; i < count; ++i)
            {
                if (sessionInfo[i].State == WTSListen)
                    continue;
#if _MSC_VER > 1600
                printf("SystemAccountProcess (SessionId: %d, pWinStationName: %Ts, State: %d)\n",
#else
#ifdef _UNICODE
                wprintf(L"SystemAccountProcess (SessionId: %d, pWinStationName: %s, State: %d)\n",
#else
                printf("SystemAccountProcess (SessionId: %d, pWinStationName: %s, State: %d)\n",
#endif
#endif
                       sessionInfo[i].SessionId, sessionInfo[i].pWinStationName, sessionInfo[i].State);

                Win32Ex::System::SystemAccountProcess process(sessionInfo[i].SessionId, "CMD /C QUERY SESSION");
                ret = process.Run();
                if (!ret)
                {
                    printf("Failed to SystemAccountProcess::Run(%d) : %d\n", sessionInfo[i].SessionId, GetLastError());
                }
                EXPECT_EQ(ret, TRUE);
            }

            WTSFreeMemory(sessionInfo);
        }
        Wow64RevertWow64FsRedirection(OldValue);
    }
}

#include <Security\Token.hpp>

TEST(ProcessTest, CreateUserAccountProcessTest)
{
    PVOID OldValue = NULL;
    Wow64DisableWow64FsRedirection(&OldValue);

    PWTS_SESSION_INFO sessionInfo = NULL;
    DWORD count = 0;
    if (WTSEnumerateSessions(WTS_CURRENT_SERVER, 0, 1, &sessionInfo, &count))
    {
        BOOL isLocalSystem = IsLocalSystemToken(GetCurrentProcessToken());
        BOOL assignPrimaryTokenAcquired =
            Win32Ex::Security::Token::Current().IsAcquired(Win32Ex::Security::SeAssignPrimaryTokenPrivilege);

        BOOL ret = FALSE;
        PROCESS_INFORMATION pi;
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(si);
        PTSTR cmd = _tcsdup(TEXT("CMD /C WHOAMI /ALL"));
        if (cmd)
        {
            for (DWORD i = 0; i < count; ++i)
            {
                if (sessionInfo[i].State == WTSListen)
                    continue;
#if _MSC_VER > 1600
                printf("CreateUserAccountProcess (SessionId: %d, pWinStationName: %Ts, State: %d)\n",
#else
#ifdef _UNICODE
                wprintf(L"CreateUserAccountProcess (SessionId: %d, pWinStationName: %s, State: %d)\n",
#else
                printf("CreateUserAccountProcess (SessionId: %d, pWinStationName: %s, State: %d)\n",
#endif
#endif
                       sessionInfo[i].SessionId, sessionInfo[i].pWinStationName, sessionInfo[i].State);
                ret = CreateUserAccountProcess(sessionInfo[i].SessionId, NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL,
                                               &si, &pi);
                if (ret)
                {
                    WaitForSingleObject(pi.hProcess, INFINITE);
                }
                else
                {
                    printf("Failed to CreateUserAccountProcess(%d) : %d\n", sessionInfo[i].SessionId, GetLastError());
                }

                if (isLocalSystem || assignPrimaryTokenAcquired ||
                    WTSGetActiveConsoleSessionId() == sessionInfo[i].SessionId)
                {
                    EXPECT_EQ(ret, TRUE);
                }
            }

            WTSFreeMemory(sessionInfo);
        }
        free(cmd);
    }
    Wow64RevertWow64FsRedirection(OldValue);
}

TEST(ProcessTest, CreateSystemAccountProcessTest)
{
    if (IsUserAdmin())
    {
        PVOID OldValue = NULL;
        Wow64DisableWow64FsRedirection(&OldValue);

        PWTS_SESSION_INFO sessionInfo = NULL;
        DWORD count = 0;
        if (WTSEnumerateSessions(WTS_CURRENT_SERVER, 0, 1, &sessionInfo, &count))
        {
            BOOL ret = FALSE;
            PROCESS_INFORMATION pi;
            STARTUPINFO si;
            ZeroMemory(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(si);
            PTSTR cmd = _tcsdup(TEXT("CMD /C QUERY SESSION"));
            if (cmd)
            {
                for (DWORD i = 0; i < count; ++i)
                {
                    if (sessionInfo[i].State == WTSListen)
                        continue;
#if _MSC_VER > 1600
                    printf("CreateSystemAccountProcess (SessionId: %d, pWinStationName: %Ts, State: %d)\n",
#else
#ifdef _UNICODE
                    wprintf(L"CreateSystemAccountProcess (SessionId: %d, pWinStationName: %s, State: %d)\n",
#else
                    printf("CreateSystemAccountProcess (SessionId: %d, pWinStationName: %s, State: %d)\n",
#endif
#endif
                           sessionInfo[i].SessionId, sessionInfo[i].pWinStationName, sessionInfo[i].State);
                    ret = CreateSystemAccountProcess(sessionInfo[i].SessionId, NULL, cmd, NULL, NULL, FALSE, 0, NULL,
                                                     NULL, &si, &pi);
                    if (ret)
                    {
                        WaitForSingleObject(pi.hProcess, INFINITE);
                    }
                    else
                    {
                        printf("Failed to CreateSystemAccountProcess(%d) : %d\n", sessionInfo[i].SessionId,
                               GetLastError());
                    }
                    EXPECT_EQ(ret, TRUE);
                }

                WTSFreeMemory(sessionInfo);
            }
            free(cmd);
        }
        Wow64RevertWow64FsRedirection(OldValue);
    }
}

/**
 * @brief Test a C Code
 *
 */
extern "C"
{
    extern BOOL CreateUserAccountProcessTestC();
    extern BOOL CreateSystemAccountProcessTestC();
}

TEST(ProcessTest, CreateUserAccountProcessTestC)
{
    EXPECT_TRUE(CreateUserAccountProcessTestC() == TRUE);
}

TEST(ProcessTest, CreateSystemAccountProcessTestC)
{
    EXPECT_TRUE(CreateSystemAccountProcessTestC() == TRUE);
}