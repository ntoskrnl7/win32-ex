#include <Win32Ex/System/Process.hpp>
#include <gtest/gtest.h>
#include <stdio.h>
#include <tchar.h>

#if _MSC_VER < 1600
#define _STD_NS_ std::tr1
#else
#define _STD_NS_ std
#endif

// for MSYS2 (MSYS)
#ifndef _tcsdup
#ifdef UNICODE
#define _tcsdup wcsdup
#else
#define _tcsdup strdup
#endif
#endif

TEST(ProcessTest, ThisProcess)
{
    Win32Ex::String path = Win32Ex::ThisProcess::ExecutablePath();
    EXPECT_EQ(path, Win32Ex::ThisProcess::ExecutablePath());
    path = Win32Ex::ThisProcess::CurrentDirectory();
    EXPECT_EQ(path, Win32Ex::ThisProcess::CurrentDirectory());
}

TEST(ProcessTest, ProcessTAll)
{
#if defined(__cpp_range_based_for)
    for (auto process : Win32Ex::System::ProcessT<>::All())
#else
    // clang-format off
    for each (const Win32Ex::SharedPtr<Win32Ex::System::ProcessT<>> &process in Win32Ex::System::ProcessT<>::All())
#endif
    {
        if (process)
        {
            if (process->IsAttached())
            {
#if defined(UNICODE)
                std::wcout << L"[Attached] PID :" << process->Id() << L"\t\tPATH : " << process->ExecutablePath()
                           << L'\n';
#else
                std::cout << "[Attached] PID :" << process->Id() << "\t\tPATH : " << process->ExecutablePath() << '\n';
#endif
            }
            else
            {
#if defined(UNICODE)
                std::wcout << L"PID :" << process->Id() << L"\t\tPATH : " << process->ExecutablePath() << L'\n';
#else
                std::cout << "PID :" << process->Id() << "\t\tPATH : " << process->ExecutablePath() << '\n';
#endif
            }
        }
    }
    // clang-format on
}

TEST(ProcessTest, ProcessAll)
{
#if defined(__cpp_range_based_for)
    for (auto process : Win32Ex::System::Process::All())
#else
    // clang-format off
    for each (const Win32Ex::SharedPtr<Win32Ex::System::Process> &process in Win32Ex::System::Process::All())
#endif
    {
        if (process)
        {
            if (process->IsAttached())
                std::cout << "[Attached] PID :" << process->Id() << "\t\tPATH : " << process->ExecutablePath() << '\n';
            else
                std::cout << "PID :" << process->Id() << "\t\tPATH : " << process->ExecutablePath() << '\n';
        }
    }
    // clang-format on
}

TEST(ProcessTest, ProcessWAll)
{
#if defined(__cpp_range_based_for)
    for (auto process : Win32Ex::System::ProcessW::All())
#else
    // clang-format off
    for each (const Win32Ex::SharedPtr<Win32Ex::System::ProcessW> &process in Win32Ex::System::ProcessW::All())
#endif
    {
        if (process)
        {
            if (process->IsAttached())
                std::wcout << "[Attached] PID :" << process->Id() << "\t\tPATH : " << process->ExecutablePath() << '\n';
            else
                std::wcout << "PID :" << process->Id() << "\t\tPATH : " << process->ExecutablePath() << '\n';
        }
    }
    // clang-format on
}

TEST(ProcessTest, Parent)
{
    Win32Ex::System::Process parent = Win32Ex::ThisProcess::Parent();
    while (parent.IsValid())
    {
        EXPECT_STREQ(parent.ExecutablePath().c_str(), Win32Ex::System::Process(parent.Id()).ExecutablePath().c_str());
        if (parent.IsAttached())
            parent.Wait(Win32Ex::Duration(100));
        parent = parent.Parent();
    }
}

TEST(ProcessTest, ParentW)
{
    Win32Ex::System::ProcessW parent = Win32Ex::ThisProcess::ParentW();
    while (parent.IsValid())
    {
        EXPECT_STREQ(parent.ExecutablePath().c_str(), Win32Ex::System::ProcessW(parent.Id()).ExecutablePath().c_str());
        if (parent.IsAttached())
            parent.Wait(Win32Ex::Duration(100));
        parent = parent.Parent();
    }
}

TEST(ProcessTest, ParentT)
{
#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
    Win32Ex::System::ProcessT<> parent = Win32Ex::ThisProcess::ParentT();
#else
    Win32Ex::System::ProcessT<> parent = Win32Ex::ThisProcess::ParentT<Win32Ex::StringT>();
#endif
    while (parent.IsValid())
    {
        EXPECT_STREQ(parent.ExecutablePath().c_str(),
                     Win32Ex::System::ProcessT<>(parent.Id()).ExecutablePath().c_str());
        if (parent.IsAttached())
            parent.Wait(Win32Ex::Duration(100));
        parent = parent.Parent();
    }
}

TEST(ProcessTest, Clone)
{
    Win32Ex::System::Process parent = Win32Ex::ThisProcess::Parent();
    Win32Ex::System::Process dup = parent.Clone();
    EXPECT_EQ(dup.Id(), parent.Id());
    EXPECT_STREQ(dup.ExecutablePath().c_str(), parent.ExecutablePath().c_str());

    {
        Win32Ex::System::SystemAccountProcess process("notepad");
        dup = process.Clone();
        EXPECT_STREQ(dup.ExecutablePath().c_str(), process.ExecutablePath().c_str());
    }
    EXPECT_STREQ(dup.ExecutablePath().c_str(), "notepad");

    {
        Win32Ex::System::UserAccountProcess process("notepad");
        dup = process.Clone();
        EXPECT_STREQ(dup.ExecutablePath().c_str(), process.ExecutablePath().c_str());
    }
    EXPECT_STREQ(dup.ExecutablePath().c_str(), "notepad");

    {
        Win32Ex::System::ElevatedProcess process("notepad");
        dup = process.Clone();
        EXPECT_STREQ(dup.ExecutablePath().c_str(), process.ExecutablePath().c_str());
    }
    EXPECT_STREQ(dup.ExecutablePath().c_str(), "notepad");
}

TEST(ProcessTest, RunInvalidProcess)
{
    Win32Ex::System::UserAccountProcess process("!@#$test");
    EXPECT_FALSE(process.Run());
}

TEST(ProcessTest, AttachByProcessId)
{
    Win32Ex::System::Process process(GetCurrentProcessId());
    EXPECT_TRUE(process.IsAttached());
    EXPECT_STRCASEEQ(process.ExecutablePath().c_str(), Win32Ex::ThisProcess::ExecutablePath().c_str());
}

TEST(ProcessTest, AttachByHandle)
{
    Win32Ex::System::Process process(Win32Ex::System::ProcessHandle::FromHANDLE(GetCurrentProcess()));
    EXPECT_TRUE(process.IsAttached());
    EXPECT_STRCASEEQ(process.ExecutablePath().c_str(), Win32Ex::ThisProcess::ExecutablePath().c_str());

    process.Attach(Win32Ex::System::ProcessHandle::FromHANDLE(GetCurrentProcess()));
    EXPECT_TRUE(process.IsAttached());
    EXPECT_STRCASEEQ(process.ExecutablePath().c_str(), Win32Ex::ThisProcess::ExecutablePath().c_str());
}

TEST(ProcessTest, RunnableProcess)
{
    PVOID oldValue = NULL;
    Wow64DisableWow64FsRedirection(&oldValue);

    Win32Ex::System::UserAccountProcess process("CMD", "/C WHOAMI");
    Win32Ex::System::RunnableProcess &runnable = process;
    EXPECT_TRUE(runnable.Run());

    if (Win32Ex::ThisProcess::IsAdmin())
    {
        Win32Ex::System::SystemAccountProcess process("CMD", "/C WHOAMI");
        Win32Ex::System::RunnableProcess &runnable = process;
        EXPECT_TRUE(runnable.Run());
    }

    {
        Win32Ex::System::ElevatedProcess process("CMD.exe", "/C WHOAMI");
        Win32Ex::System::RunnableProcess &runnable = process;
        runnable.Run(); // EXPECT_TRUE(runnable.Run());
    }

    Wow64RevertWow64FsRedirection(oldValue);
}

#if !defined(__cpp_lambdas)
void onEnter(Win32Ex::System::RunnableProcess *process)
{
    Sleep(500);
    process->Exit();
}
void onEnterW(Win32Ex::System::RunnableProcessW *process)
{
    Sleep(500);
    process->Exit();
}
void onEnterT(Win32Ex::System::RunnableProcessT<> *process)
{
    Sleep(500);
    process->Exit();
}
void onEnterSystemProcess(Win32Ex::System::SystemAccountProcess *process, bool *isAdmin)
{
    Sleep(500);
    *isAdmin = process->IsAdmin();
    process->Exit();
}
void onEnterSystemProcessW(Win32Ex::System::SystemAccountProcessW *process, bool *isAdmin)
{
    Sleep(500);
    *isAdmin = process->IsAdmin();
    process->Exit();
}
void onEnterSystemProcessT(Win32Ex::System::SystemAccountProcessT *process, bool *isAdmin)
{
    Sleep(500);
    *isAdmin = process->IsAdmin();
    process->Exit();
}
void onExit(bool *terminated)
{
    *terminated = true;
}
void onError(bool *terminated, DWORD, const std::exception &)
{
    *terminated = true;
}
#endif

TEST(ProcessTest, UserAccountProcess)
{
    {
        PVOID oldValue = NULL;
        Wow64DisableWow64FsRedirection(&oldValue);

        Win32Ex::System::UserAccountProcess process("CMD");
        EXPECT_TRUE(process.Run("/C QUERY SESSION"));

        Wow64RevertWow64FsRedirection(oldValue);
    }

    Win32Ex::System::UserAccountProcess process("notepad.exe");
    bool terminated = false;
#if defined(__cpp_lambdas)
    process
        .OnEnter([&process]() {
            Sleep(500);
            EXPECT_FALSE(process.IsAdmin());
            process.Exit();
        })
        .OnExit([&terminated]() { terminated = true; })
        .OnError([&terminated](DWORD, const std::exception &) { terminated = true; });
#else
    process.OnEnter(_STD_NS_::bind(onEnter, &process))
        .OnExit(_STD_NS_::bind(onExit, &terminated))
        .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif

    EXPECT_FALSE(terminated);
    EXPECT_TRUE(process.Run());
    EXPECT_TRUE(terminated);
}

TEST(ProcessTest, UserAccountProcessW)
{
    {
        PVOID oldValue = NULL;
        Wow64DisableWow64FsRedirection(&oldValue);

        Win32Ex::System::UserAccountProcessW process(L"CMD", L"/C QUERY SESSION");
        EXPECT_TRUE(process.Run());

        Wow64RevertWow64FsRedirection(oldValue);
    }
    Win32Ex::System::UserAccountProcessW process(L"notepad.exe");
    bool terminated = false;
#if defined(__cpp_lambdas)
    process
        .OnEnter([&process]() {
            Sleep(500);
            EXPECT_FALSE(process.IsAdmin());
            process.Exit();
        })
        .OnExit([&terminated]() { terminated = true; })
        .OnError([&terminated](DWORD, const std::exception &) { terminated = true; });
#else
    process.OnEnter(_STD_NS_::bind(onEnterW, &process))
        .OnExit(_STD_NS_::bind(onExit, &terminated))
        .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif

    EXPECT_FALSE(terminated);
    EXPECT_TRUE(process.Run());
    EXPECT_TRUE(terminated);
}

TEST(ProcessTest, UserAccountProcessT)
{
    {
        PVOID oldValue = NULL;
        Wow64DisableWow64FsRedirection(&oldValue);

        Win32Ex::System::UserAccountProcessT process(TEXT("CMD"), TEXT("/C QUERY SESSION"));
        EXPECT_TRUE(process.Run());

        Wow64RevertWow64FsRedirection(oldValue);
    }
    Win32Ex::System::UserAccountProcessT process(TEXT("notepad.exe"));
    bool terminated = false;
#if defined(__cpp_lambdas)
    process
        .OnEnter([&process]() {
            Sleep(500);
            EXPECT_FALSE(process.IsAdmin());
            process.Exit();
        })
        .OnExit([&terminated]() { terminated = true; })
        .OnError([&terminated](DWORD, const std::exception &) { terminated = true; });
#else
    process.OnEnter(_STD_NS_::bind(onEnterT, &process))
        .OnExit(_STD_NS_::bind(onExit, &terminated))
        .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif

    EXPECT_FALSE(terminated);
    EXPECT_TRUE(process.Run());
    EXPECT_TRUE(terminated);
}

TEST(ProcessTest, ElevatedProcess)
{
    Win32Ex::System::ElevatedProcess process("notepad.exe");
    bool terminated = false;
#if defined(__cpp_lambdas)
    process
        .OnEnter([&process]() {
            Sleep(500);
            EXPECT_FALSE(process.IsAdmin());
            process.Exit();
        })
        .OnExit([&terminated]() { terminated = true; })
        .OnError([&terminated](DWORD, const std::exception &) { terminated = true; });
#else
    process.OnEnter(_STD_NS_::bind(onEnter, &process))
        .OnExit(_STD_NS_::bind(onExit, &terminated))
        .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif
    EXPECT_FALSE(terminated);
    process.Run(); // EXPECT_TRUE(process.Run());
    EXPECT_TRUE(terminated);
}

TEST(ProcessTest, ElevatedProcessW)
{
    Win32Ex::System::ElevatedProcessW process(L"notepad.exe");
    bool terminated = false;
#if defined(__cpp_lambdas)
    process
        .OnEnter([&process]() {
            Sleep(500);
            EXPECT_FALSE(process.IsAdmin());
            process.Exit();
        })
        .OnExit([&terminated]() { terminated = true; })
        .OnError([&terminated](DWORD, const std::exception &) { terminated = true; });
#else
    process.OnEnter(_STD_NS_::bind(onEnterW, &process))
        .OnExit(_STD_NS_::bind(onExit, &terminated))
        .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif
    EXPECT_FALSE(terminated);
    process.Run(); // EXPECT_TRUE(process.Run());
    EXPECT_TRUE(terminated);
}

TEST(ProcessTest, ElevatedProcessT)
{
    Win32Ex::System::ElevatedProcessT<> process(TEXT("notepad.exe"));
    bool terminated = false;
#if defined(__cpp_lambdas)
    process
        .OnEnter([&process]() {
            Sleep(500);
            EXPECT_FALSE(process.IsAdmin());
            process.Exit();
        })
        .OnExit([&terminated]() { terminated = true; })
        .OnError([&terminated](DWORD, const std::exception &) { terminated = true; });
#else
    process.OnEnter(_STD_NS_::bind(onEnterT, &process))
        .OnExit(_STD_NS_::bind(onExit, &terminated))
        .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif
    EXPECT_FALSE(terminated);
    process.Run(); // EXPECT_TRUE(process.Run());
    EXPECT_TRUE(terminated);
}

TEST(ProcessTest, SystemAccountProcess)
{
    if (Win32Ex::ThisProcess::IsAdmin())
    {
        {
            PVOID oldValue = NULL;
            Wow64DisableWow64FsRedirection(&oldValue);

            Win32Ex::System::SystemAccountProcess process("CMD", "/C WHOAMI");
            EXPECT_TRUE(process.Run());

            Wow64RevertWow64FsRedirection(oldValue);
        }

        Win32Ex::System::SystemAccountProcess process("notepad.exe");
        bool terminated = false;
#if defined(__cpp_lambdas)
        process
            .OnEnter([&process]() {
                Sleep(500);
                EXPECT_TRUE(process.IsAdmin());
                process.Exit();
            })
            .OnExit([&terminated]() { terminated = true; })
            .OnError([&terminated](DWORD, const std::exception &) { terminated = true; });
#else
        bool isAdmin;
        process.OnEnter(_STD_NS_::bind(onEnterSystemProcess, &process, &isAdmin))
            .OnExit(_STD_NS_::bind(onExit, &terminated))
            .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif
        EXPECT_FALSE(terminated);
        EXPECT_TRUE(process.Run());
#if !defined(__cpp_lambdas)
        EXPECT_TRUE(isAdmin);
#endif
        EXPECT_TRUE(terminated);
    }
}

TEST(ProcessTest, SystemAccountProcessW)
{
    if (Win32Ex::ThisProcess::IsAdmin())
    {
        {
            PVOID oldValue = NULL;
            Wow64DisableWow64FsRedirection(&oldValue);

            Win32Ex::System::SystemAccountProcessW process(L"CMD", L"/C WHOAMI");
            EXPECT_TRUE(process.Run());

            Wow64RevertWow64FsRedirection(oldValue);
        }

        Win32Ex::System::SystemAccountProcessW process(L"notepad.exe");
        bool terminated = false;
#if defined(__cpp_lambdas)
        process
            .OnEnter([&process]() {
                Sleep(500);
                EXPECT_TRUE(process.IsAdmin());
                process.Exit();
            })
            .OnExit([&terminated]() { terminated = true; })
            .OnError([&terminated](DWORD, const std::exception &) { terminated = true; });
#else
        bool isAdmin;
        process.OnEnter(_STD_NS_::bind(onEnterSystemProcessW, &process, &isAdmin))
            .OnExit(_STD_NS_::bind(onExit, &terminated))
            .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif
        EXPECT_FALSE(terminated);
        EXPECT_TRUE(process.Run());
#if !defined(__cpp_lambdas)
        EXPECT_TRUE(isAdmin);
#endif
        EXPECT_TRUE(terminated);
    }
}

TEST(ProcessTest, SystemAccountProcessT)
{
    if (Win32Ex::ThisProcess::IsAdmin())
    {
        {
            PVOID oldValue = NULL;
            Wow64DisableWow64FsRedirection(&oldValue);

            Win32Ex::System::SystemAccountProcessT process(TEXT("CMD"), TEXT("/C WHOAMI"));
            EXPECT_TRUE(process.Run());

            Wow64RevertWow64FsRedirection(oldValue);
        }

        Win32Ex::System::SystemAccountProcessT process(TEXT("notepad.exe"));
        bool terminated = false;
#if defined(__cpp_lambdas)
        process
            .OnEnter([&process]() {
                Sleep(500);
                EXPECT_TRUE(process.IsAdmin());
                process.Exit();
            })
            .OnExit([&terminated]() { terminated = true; })
            .OnError([&terminated](DWORD, const std::exception &) { terminated = true; });
#else
        bool isAdmin;
        process.OnEnter(_STD_NS_::bind(onEnterSystemProcessT, &process, &isAdmin))
            .OnExit(_STD_NS_::bind(onExit, &terminated))
            .OnError(_STD_NS_::bind(onError, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif
        EXPECT_FALSE(terminated);
        EXPECT_TRUE(process.Run());
#if !defined(__cpp_lambdas)
        EXPECT_TRUE(isAdmin);
#endif
        EXPECT_TRUE(terminated);
    }
}

#if !defined(__cpp_lambdas)
void onExitCV(PCONDITION_VARIABLE cv, bool *terminated)
{
    WakeConditionVariable(cv);
    *terminated = true;
}
void onErrorCV(PCONDITION_VARIABLE cv, bool *terminated, DWORD, const std::exception &)
{
    WakeConditionVariable(cv);
    *terminated = true;
}
#endif

TEST(ProcessTest, ElevatedProcessRunAsync)
{
    CRITICAL_SECTION cs;
    CONDITION_VARIABLE cv;
    InitializeConditionVariable(&cv);
    InitializeCriticalSection(&cs);

    Win32Ex::System::ElevatedProcess process("notepad.exe");
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
        .OnError([&cv, &terminated](DWORD, const std::exception &) {
            terminated = true;
            WakeConditionVariable(&cv);
        });
#else
    process.OnEnter(_STD_NS_::bind(onEnter, &process))
        .OnExit(_STD_NS_::bind(onExitCV, &cv, &terminated))
        .OnError(_STD_NS_::bind(onErrorCV, &cv, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif
    EXPECT_FALSE(terminated);
    Win32Ex::Waitable waitable = process.RunAsync();
    if (!terminated)
        SleepConditionVariableCS(&cv, &cs, INFINITE);

    EXPECT_TRUE(terminated);
    EXPECT_FALSE(waitable);
}

TEST(ProcessTest, UserAccountProcessRunAsync)
{
    CRITICAL_SECTION cs;
    CONDITION_VARIABLE cv;
    InitializeConditionVariable(&cv);
    InitializeCriticalSection(&cs);

    Win32Ex::System::UserAccountProcess process("notepad.exe");
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
        .OnError([&cv, &terminated](DWORD, const std::exception &) {
            terminated = true;
            WakeConditionVariable(&cv);
        });
#else
    process.OnEnter(_STD_NS_::bind(onEnter, &process))
        .OnExit(_STD_NS_::bind(onExitCV, &cv, &terminated))
        .OnError(_STD_NS_::bind(onErrorCV, &cv, &terminated, _STD_NS_::placeholders::_1, _STD_NS_::placeholders::_2));
#endif
    EXPECT_FALSE(terminated);
    Win32Ex::Waitable waitable = process.RunAsync();
    if (!terminated)
        SleepConditionVariableCS(&cv, &cs, INFINITE);

    EXPECT_TRUE(terminated);
    EXPECT_FALSE(waitable);
}

TEST(ProcessTest, UserAccountProcessAllSessions)
{
    PVOID oldValue = NULL;
    Wow64DisableWow64FsRedirection(&oldValue);

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
#ifdef _UNICODE
            wprintf(L"UserAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("UserAccountProcess (SessionId: %lu, pWinStationName: %Ts, State: %d)\n",
#endif
#else
#ifdef _UNICODE
            wprintf(L"UserAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("UserAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#endif
#endif
                    sessionInfo[i].SessionId, sessionInfo[i].pWinStationName, sessionInfo[i].State);

            Win32Ex::System::UserAccountProcess process(sessionInfo[i].SessionId, "CMD", "/C QUERY SESSION");
            ret = process.Run();
            if (!ret)
                printf("Failed to UserAccountProcess::Run(%lu) : %lu\n", sessionInfo[i].SessionId, GetLastError());

            if (isLocalSystem || WTSGetActiveConsoleSessionId() == sessionInfo[i].SessionId)
                EXPECT_EQ(ret, TRUE);
        }

        WTSFreeMemory(sessionInfo);
    }
    Wow64RevertWow64FsRedirection(oldValue);
}

TEST(ProcessTest, UserAccountProcessWAllSessions)
{
    PVOID oldValue = NULL;
    Wow64DisableWow64FsRedirection(&oldValue);

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
#ifdef _UNICODE
            wprintf(L"UserAccountProcessW (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("UserAccountProcessW (SessionId: %lu, pWinStationName: %Ts, State: %d)\n",
#endif
#else
#ifdef _UNICODE
            wprintf(L"UserAccountProcessW (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("UserAccountProcessW (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#endif
#endif
                    sessionInfo[i].SessionId, sessionInfo[i].pWinStationName, sessionInfo[i].State);

            Win32Ex::System::UserAccountProcessW process(sessionInfo[i].SessionId, L"CMD", L"/C QUERY SESSION");
            ret = process.Run();
            if (!ret)
                printf("Failed to UserAccountProcessW::Run(%lu) : %lu\n", sessionInfo[i].SessionId, GetLastError());

            if (isLocalSystem || WTSGetActiveConsoleSessionId() == sessionInfo[i].SessionId)
                EXPECT_EQ(ret, TRUE);
        }

        WTSFreeMemory(sessionInfo);
    }
    Wow64RevertWow64FsRedirection(oldValue);
}

TEST(ProcessTest, UserAccountProcessTAllSessions)
{
    PVOID oldValue = NULL;
    Wow64DisableWow64FsRedirection(&oldValue);

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
#ifdef _UNICODE
            wprintf(L"UserAccountProcessT (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("UserAccountProcessT (SessionId: %lu, pWinStationName: %Ts, State: %d)\n",
#endif
#else
#ifdef _UNICODE
            wprintf(L"UserAccountProcessT (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("UserAccountProcessT (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#endif
#endif
                    sessionInfo[i].SessionId, sessionInfo[i].pWinStationName, sessionInfo[i].State);

            Win32Ex::System::UserAccountProcessT process(sessionInfo[i].SessionId, TEXT("CMD"),
                                                         TEXT("/C QUERY SESSION"));
            ret = process.Run();
            if (!ret)
                printf("Failed to UserAccountProcessT::Run(%lu) : %lu\n", sessionInfo[i].SessionId, GetLastError());

            if (isLocalSystem || WTSGetActiveConsoleSessionId() == sessionInfo[i].SessionId)
                EXPECT_EQ(ret, TRUE);
        }

        WTSFreeMemory(sessionInfo);
    }
    Wow64RevertWow64FsRedirection(oldValue);
}

TEST(ProcessTest, SystemAccountProcessAllSessions)
{
    if (!Win32Ex::ThisProcess::IsAdmin())
        return;

    PVOID oldValue = NULL;
    Wow64DisableWow64FsRedirection(&oldValue);

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
#ifdef _UNICODE
            wprintf(L"SystemAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("SystemAccountProcess (SessionId: %lu, pWinStationName: %Ts, State: %d)\n",
#endif
#else
#ifdef _UNICODE
            wprintf(L"SystemAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("SystemAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#endif
#endif
                    sessionInfo[i].SessionId, sessionInfo[i].pWinStationName, sessionInfo[i].State);

            Win32Ex::System::SystemAccountProcess process(sessionInfo[i].SessionId, "CMD", "/C QUERY SESSION");
            ret = process.Run();
            if (!ret)
                printf("Failed to SystemAccountProcess::Run(%lu) : %lu\n", sessionInfo[i].SessionId, GetLastError());

            if (isLocalSystem || WTSGetActiveConsoleSessionId() == sessionInfo[i].SessionId)
                EXPECT_EQ(ret, TRUE);
        }

        WTSFreeMemory(sessionInfo);
    }
    Wow64RevertWow64FsRedirection(oldValue);
}

TEST(ProcessTest, SystemAccountProcessWAllSessions)
{
    if (!Win32Ex::ThisProcess::IsAdmin())
        return;

    PVOID oldValue = NULL;
    Wow64DisableWow64FsRedirection(&oldValue);

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
#ifdef _UNICODE
            wprintf(L"SystemAccountProcessW (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("SystemAccountProcessW (SessionId: %lu, pWinStationName: %Ts, State: %d)\n",
#endif
#else
#ifdef _UNICODE
            wprintf(L"SystemAccountProcessW (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("SystemAccountProcessW (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#endif
#endif
                    sessionInfo[i].SessionId, sessionInfo[i].pWinStationName, sessionInfo[i].State);

            Win32Ex::System::SystemAccountProcessW process(sessionInfo[i].SessionId, L"CMD", L"/C QUERY SESSION");
            ret = process.Run();
            if (!ret)
                printf("Failed to SystemAccountProcessW::Run(%lu) : %lu\n", sessionInfo[i].SessionId, GetLastError());

            if (isLocalSystem || WTSGetActiveConsoleSessionId() == sessionInfo[i].SessionId)
                EXPECT_EQ(ret, TRUE);
        }

        WTSFreeMemory(sessionInfo);
    }
    Wow64RevertWow64FsRedirection(oldValue);
}

TEST(ProcessTest, SystemAccountProcessTAllSessions)
{
    if (!Win32Ex::ThisProcess::IsAdmin())
        return;

    PVOID oldValue = NULL;
    Wow64DisableWow64FsRedirection(&oldValue);

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
#ifdef _UNICODE
            wprintf(L"SystemAccountProcessT (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("SystemAccountProcessT (SessionId: %lu, pWinStationName: %Ts, State: %d)\n",
#endif
#else
#ifdef _UNICODE
            wprintf(L"SystemAccountProcessT (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
            printf("SystemAccountProcessT (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#endif
#endif
                    sessionInfo[i].SessionId, sessionInfo[i].pWinStationName, sessionInfo[i].State);

            Win32Ex::System::SystemAccountProcessT process(sessionInfo[i].SessionId, TEXT("CMD"),
                                                           TEXT("/C QUERY SESSION"));
            ret = process.Run();
            if (!ret)
                printf("Failed to SystemAccountProcessT::Run(%lu) : %lu\n", sessionInfo[i].SessionId, GetLastError());

            if (isLocalSystem || WTSGetActiveConsoleSessionId() == sessionInfo[i].SessionId)
                EXPECT_EQ(ret, TRUE);
        }

        WTSFreeMemory(sessionInfo);
    }
    Wow64RevertWow64FsRedirection(oldValue);
}

#include <Win32Ex/Security/Token.hpp>

TEST(ProcessTest, CreateUserAccountProcessAllSessions)
{
    PVOID oldValue = NULL;
    Wow64DisableWow64FsRedirection(&oldValue);

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

        PTSTR cmd = _tcsdup(TEXT("CMD /C QUERY SESSION"));
        if (cmd)
        {
            for (DWORD i = 0; i < count; ++i)
            {
                if (sessionInfo[i].State == WTSListen)
                    continue;
#if _MSC_VER > 1600
#ifdef _UNICODE
                wprintf(L"CreateUserAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
                printf("CreateUserAccountProcess (SessionId: %lu, pWinStationName: %Ts, State: %d)\n",
#endif
#else
#ifdef _UNICODE
                wprintf(L"CreateUserAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
                printf("CreateUserAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
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
                    printf("Failed to CreateUserAccountProcess(%lu) : %lu\n", sessionInfo[i].SessionId, GetLastError());
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
    Wow64RevertWow64FsRedirection(oldValue);
}

TEST(ProcessTest, CreateSystemAccountProcessAllSessions)
{
    if (IsUserAdmin())
    {
        PVOID oldValue = NULL;
        Wow64DisableWow64FsRedirection(&oldValue);

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
#ifdef _UNICODE
                    wprintf(L"CreateSystemAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
                    printf("CreateSystemAccountProcess (SessionId: %lu, pWinStationName: %Ts, State: %d)\n",
#endif
#else
#ifdef _UNICODE
                    wprintf(L"CreateSystemAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
#else
                    printf("CreateSystemAccountProcess (SessionId: %lu, pWinStationName: %s, State: %d)\n",
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
                        printf("Failed to CreateSystemAccountProcess(%lu) : %lu\n", sessionInfo[i].SessionId,
                               GetLastError());
                    }

                    // GitHub Action의 Windows 운영체제에서는 세션0으로 System Process를 생성하는 것이 실패합니다.
                    // 추후 원인 파악이 필요합니다. :-(
                    if (!((((sessionInfo[i].SessionId == 0) && GetLastError() == ERROR_ACCESS_DENIED))))
                    {
                        EXPECT_EQ(ret, TRUE);
                    }
                }

                WTSFreeMemory(sessionInfo);
            }
            free(cmd);
        }
        Wow64RevertWow64FsRedirection(oldValue);
    }
}

/**
 * @brief Test a C Code
 *
 */
extern "C"
{
    extern BOOL CreateUserAccountProcessAllSessionsC();
    extern BOOL CreateSystemAccountProcessAllSessionsC();
}

TEST(ProcessTest, CreateUserAccountProcessAllSessionsC)
{
    EXPECT_TRUE(CreateUserAccountProcessAllSessionsC() == TRUE);
}

TEST(ProcessTest, CreateSystemAccountProcessAllSessionsC)
{
    EXPECT_TRUE(CreateSystemAccountProcessAllSessionsC() == TRUE);
}