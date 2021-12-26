#include <Win32Ex/System/Process.hpp>

#define TWIN32EX_USE_SERVICE_SIMULATE_MODE
#include <Win32Ex/System/Service.hpp>
#include <gtest/gtest.h>

#ifdef _INC__MINGW_H
static const std::string testSvcPath = []() -> std::string {
    HMODULE hModule = GetModuleHandleA("msys-2.0.dll");
    if (hModule == NULL)
    {
        hModule = GetModuleHandleA("libgcc_s_seh-1.dll");
    }
    if (hModule == NULL)
    {
        hModule = GetModuleHandleA("libstdc++-6.dll");
    }
    if (hModule == NULL)
    {
        hModule = GetModuleHandleA("libc++.dll");
    }
    if (hModule == NULL)
    {
        hModule = GetModuleHandleA("libwinpthread-1.dll");
    }
    if (hModule == NULL)
    {
        std::cerr << "Failed to GetModuleHandleA\n";
        return "";
    }

    std::string path(MAX_PATH, '\0');
    size_t returnSize = GetModuleFileNameA(hModule, &path[0], (DWORD)path.size());
    for (;;)
    {
        if (returnSize < path.size())
        {
            path.resize(returnSize);
            break;
        }
        else
        {
            path.resize(returnSize + MAX_PATH);
            returnSize = GetModuleFileNameA(hModule, &path[0], (DWORD)path.size());
            path.resize(returnSize);
        }
        if (GetLastError() == ERROR_SUCCESS)
        {
            break;
        }
    }
    size_t pos = path.find_last_of('\\');
    if (pos != std::string::npos)
        path.resize(pos + 1);

    const std::string &fileName = Win32Ex::ThisProcess::ExecutablePath();
    pos = fileName.find_last_of('\\');
    if (pos == std::string::npos)
    {
        std::cerr << "Invalid file name : " << fileName << '\n';
        return "";
    }
    path.append(&fileName[pos + 1]);
    if (fileName == path)
    {
        return "";
    }
    if (!CopyFileA(fileName.c_str(), path.c_str(), FALSE))
    {
        std::cerr << "Failed to CopyFileA (" << path << ")\n";
        return "";
    }
    std::cout << "file name : " << path << '\n';
    return path;
}();

int __tmp___ = atexit([]() {
    if (!testSvcPath.empty())
    {
        if (!DeleteFileA(testSvcPath.c_str()))
        {
            std::cerr << "Failed to DeleteFileA (" << testSvcPath << ")\n";
        }
    }
});
#endif

#include "TestService.h"
extern Win32Ex::System::ServiceConfig TestServiceConfig;
typedef Win32Ex::System::Service<TestServiceConfig> TestService;

extern Win32Ex::System::ServiceConfig Test2ServiceConfig;
typedef Win32Ex::System::Service<Test2ServiceConfig> Test2Service;

extern Win32Ex::System::ServiceConfigW TestServiceConfigW;
typedef Win32Ex::System::ServiceW<TestServiceConfigW> TestServiceW;

extern Win32Ex::System::ServiceConfigW Test2ServiceConfigW;
typedef Win32Ex::System::ServiceW<Test2ServiceConfigW> Test2ServiceW;

#ifndef TWIN32EX_USE_SERVICE_SIMULATE_MODE
TEST(ServiceTest, InvalidServiceRun)
{
    TestService &svc = TestService::Instance();
    EXPECT_FALSE(svc.Run());
}
#else

void OnTestSvcStart()
{
    TestServiceConfig.Stop();
}
void OnTest2SvcStart()
{
    TestServiceConfigW.Stop();
}

TEST(ServiceTest, SimulateMode)
{
    TestService &svc = TestService::Instance();
    EXPECT_TRUE(svc.OnStart(OnTestSvcStart).Run());
}

TEST(ServiceTest, SimulateModeW)
{
    TestServiceW &svc = TestServiceW::Instance();
    EXPECT_TRUE(svc.OnStart(OnTest2SvcStart).Run());
}
#endif

TEST(ServiceTest, ServiceInstall)
{
    if (!IsUserAdmin())
        return;
    Win32Ex::String path =
#ifdef _INC__MINGW_H
        testSvcPath.empty() ? TestServiceConfig.BinaryPathName() : testSvcPath;
#else
        TestServiceConfig.BinaryPathName();
#endif
    path.append(" " TEST_SVC_NAME);
    EXPECT_TRUE(TestServiceConfig.Install(SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, path));
    EXPECT_TRUE(TestServiceConfig.Installed());
}

TEST(ServiceTest, ServiceStart)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfig.Installed())
        return;

    EXPECT_TRUE(TestServiceConfig.Start());

    SERVICE_STATUS ss;
    if (TestServiceConfig.QueryServiceStatus(ss))
    {
        EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
        EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
    }
}

TEST(ServiceTest, ServiceControl)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfig.Installed())
        return;

    EXPECT_TRUE(TestServiceConfig.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP));
}

TEST(ServiceTest, ServicePause)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfig.Installed())
        return;

    EXPECT_TRUE(TestServiceConfig.Pause());
}

TEST(ServiceTest, ServiceContinue)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfig.Installed())
        return;

    EXPECT_TRUE(TestServiceConfig.Continue());
}

TEST(ServiceTest, ServiceStop)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfig.Installed())
        return;

    EXPECT_TRUE(TestServiceConfig.Stop());

    SERVICE_STATUS ss;
    if (TestServiceConfig.QueryServiceStatus(ss))
    {
        EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
    }
}

TEST(ServiceTest, ServiceUninstall)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfig.Installed())
        return;

    EXPECT_TRUE(TestServiceConfig.Uninstall());
    EXPECT_FALSE(TestServiceConfig.Installed());
}

TEST(ServiceTest, ServiceWInstall)
{
    if (!IsUserAdmin())
        return;

    using namespace Win32Ex::Convert::String;

    std::wstring path =
#ifdef _INC__MINGW_H
        testSvcPath.empty() ? TestServiceConfigW.BinaryPathName() : !testSvcPath;
#else
        TestServiceConfigW.BinaryPathName();
#endif
    path.append(L" " _W(TEST_SVC_NAME) L" W");
    EXPECT_TRUE(TestServiceConfigW.Install(SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, path));
    EXPECT_TRUE(TestServiceConfigW.Installed());
}

TEST(ServiceTest, ServiceWStart)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfigW.Installed())
        return;

    EXPECT_TRUE(TestServiceConfigW.Start());

    SERVICE_STATUS ss;
    if (TestServiceConfigW.QueryServiceStatus(ss))
    {
        EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
        EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
    }
}

TEST(ServiceTest, ServiceWControl)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfigW.Installed())
        return;

    EXPECT_TRUE(TestServiceConfigW.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP));
}

TEST(ServiceTest, ServiceWPause)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfigW.Installed())
        return;

    EXPECT_TRUE(TestServiceConfigW.Pause());
}

TEST(ServiceTest, ServiceWContinue)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfig.Installed())
        return;

    EXPECT_TRUE(TestServiceConfig.Continue());
}

TEST(ServiceTest, ServiceWStop)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfigW.Installed())
        return;

    EXPECT_TRUE(TestServiceConfigW.Stop());

    SERVICE_STATUS ss;
    if (TestServiceConfigW.QueryServiceStatus(ss))
    {
        EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
    }
}

TEST(ServiceTest, ServiceWUninstall)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceConfigW.Installed())
        return;

    EXPECT_TRUE(TestServiceConfigW.Uninstall());
    EXPECT_FALSE(TestServiceConfigW.Installed());
}

TEST(ServiceTest, IsServiceMode)
{
    EXPECT_FALSE(Win32Ex::System::IsServiceMode());
}

TEST(ServiceTest, SharedServiceInstall)
{
    if (!IsUserAdmin())
        return;

    Win32Ex::String path =
#ifdef _INC__MINGW_H
        testSvcPath.empty() ? Win32Ex::ThisProcess::ExecutablePath() : testSvcPath;
#else
        Win32Ex::ThisProcess::ExecutablePath();
#endif
    path.append(" SharedService");
    EXPECT_TRUE(TestServiceConfig.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path.c_str()));
    EXPECT_TRUE(TestServiceConfig.Installed());

    EXPECT_TRUE(Test2ServiceConfig.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path.c_str()));
    EXPECT_TRUE(Test2ServiceConfig.Installed());
}

TEST(ServiceTest, SharedServiceStart)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceConfig.Installed())
    {
        EXPECT_TRUE(TestServiceConfig.Start());
        SERVICE_STATUS ss;
        if (TestServiceConfig.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }

    if (Test2ServiceConfig.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfig.Start());
        SERVICE_STATUS ss;
        if (Test2ServiceConfig.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
}

TEST(ServiceTest, SharedServiceControl)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceConfig.Installed())
    {
        EXPECT_TRUE(TestServiceConfig.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP));
    }
    if (Test2ServiceConfig.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfig.Control(TEST2_SVC_USER_CONTROL_ACCEPT_STOP));
    }
}

TEST(ServiceTest, SharedServicePause)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceConfig.Installed())
        EXPECT_TRUE(TestServiceConfig.Pause());

    if (Test2ServiceConfig.Installed())
        EXPECT_TRUE(Test2ServiceConfig.Pause());
}

TEST(ServiceTest, SharedServiceContinue)
{
    if (!IsUserAdmin())
        return;
    if (TestServiceConfig.Installed())
        EXPECT_TRUE(TestServiceConfig.Continue());

    if (Test2ServiceConfig.Installed())
        EXPECT_TRUE(Test2ServiceConfig.Continue());
}

TEST(ServiceTest, SharedServiceStop)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceConfig.Installed())
    {
        EXPECT_TRUE(TestServiceConfig.Stop());
        SERVICE_STATUS ss;
        if (TestServiceConfig.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
    if (Test2ServiceConfig.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfig.Stop());
        SERVICE_STATUS ss;
        if (Test2ServiceConfig.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
}

TEST(ServiceTest, SharedServiceUninstall)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceConfig.Installed())
    {
        EXPECT_TRUE(TestServiceConfig.Uninstall());
        EXPECT_FALSE(TestServiceConfig.Installed());
    }

    if (Test2ServiceConfig.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfig.Uninstall());
        EXPECT_FALSE(Test2ServiceConfig.Installed());
    }
}

TEST(ServiceTest, SharedServiceWInstall)
{
    if (!IsUserAdmin())
        return;

    using namespace Win32Ex::Convert::String;

    std::wstring path =
#ifdef _INC__MINGW_H
        testSvcPath.empty() ? Win32Ex::ThisProcess::ExecutablePathW() : !testSvcPath;
#else
        Win32Ex::ThisProcess::ExecutablePathW();
#endif
    path.append(L" SharedService W");

    EXPECT_TRUE(TestServiceConfigW.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path));
    EXPECT_TRUE(TestServiceConfigW.Installed());

    EXPECT_TRUE(Test2ServiceConfigW.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path));
    EXPECT_TRUE(Test2ServiceConfigW.Installed());
}

TEST(ServiceTest, SharedServiceWStart)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceConfigW.Installed())
    {
        EXPECT_TRUE(TestServiceConfigW.Start());
        SERVICE_STATUS ss;
        if (TestServiceConfigW.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }

    if (Test2ServiceConfigW.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfigW.Start());
        SERVICE_STATUS ss;
        if (Test2ServiceConfigW.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
}

TEST(ServiceTest, SharedServiceWControl)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceConfigW.Installed())
    {
        EXPECT_TRUE(TestServiceConfigW.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP));
    }
    if (Test2ServiceConfigW.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfigW.Control(TEST2_SVC_USER_CONTROL_ACCEPT_STOP));
    }
}

TEST(ServiceTest, SharedServiceWPause)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceConfigW.Installed())
        EXPECT_TRUE(TestServiceConfigW.Pause());

    if (Test2ServiceConfigW.Installed())
        EXPECT_TRUE(Test2ServiceConfigW.Pause());
}

TEST(ServiceTest, SharedServiceWContinue)
{
    if (!IsUserAdmin())
        return;
    if (TestServiceConfigW.Installed())
        EXPECT_TRUE(TestServiceConfigW.Continue());

    if (Test2ServiceConfigW.Installed())
        EXPECT_TRUE(Test2ServiceConfigW.Continue());
}

TEST(ServiceTest, SharedServiceWStop)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceConfigW.Installed())
    {
        EXPECT_TRUE(TestServiceConfigW.Stop());
        SERVICE_STATUS ss;
        if (TestServiceConfigW.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
    if (Test2ServiceConfigW.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfigW.Stop());
        SERVICE_STATUS ss;
        if (Test2ServiceConfigW.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
}

TEST(ServiceTest, SharedServiceWUninstall)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceConfigW.Installed())
    {
        EXPECT_TRUE(TestServiceConfigW.Uninstall());
        EXPECT_FALSE(TestServiceConfigW.Installed());
    }

    if (Test2ServiceConfigW.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfigW.Uninstall());
        EXPECT_FALSE(Test2ServiceConfigW.Installed());
    }
}
