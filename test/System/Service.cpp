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
extern Win32Ex::System::Service TestService;
typedef Win32Ex::System::Service::Instance<TestService> TestServiceInstance;

extern Win32Ex::System::Service Test2Service;
typedef Win32Ex::System::Service::Instance<Test2Service> Test2ServiceInstance;

extern Win32Ex::System::ServiceW TestServiceW;
typedef Win32Ex::System::ServiceW::Instance<TestServiceW> TestServiceInstanceW;

extern Win32Ex::System::ServiceW Test2ServiceW;
typedef Win32Ex::System::ServiceW::Instance<Test2ServiceW> Test2ServiceInstanceW;

#ifndef TWIN32EX_USE_SERVICE_SIMULATE_MODE
TEST(ServiceTest, InvalidServiceRun)
{
    EXPECT_FALSE(TestServiceInstance::GetInstance().Run());
}
#else
void OnTestSvcStart()
{
    TestService.Stop();
}
void OnTest2SvcStart()
{
    TestServiceW.Stop();
}

TEST(ServiceTest, SimulateMode)
{
    EXPECT_TRUE(TestServiceInstance::GetInstance().OnStart(OnTestSvcStart).Run());
}

TEST(ServiceTest, SimulateModeW)
{
    EXPECT_TRUE(TestServiceInstanceW::GetInstance().OnStart(OnTest2SvcStart).Run());
}
#endif

TEST(ServiceTest, ServiceInstall)
{
    if (!IsUserAdmin())
        return;
    Win32Ex::String path =
#ifdef _INC__MINGW_H
        testSvcPath.empty() ? TestService.BinaryPathName() : testSvcPath;
#else
        TestService.BinaryPathName();
#endif
    path.append(" " TEST_SVC_NAME);
    EXPECT_TRUE(TestService.Install(SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, path));
    EXPECT_TRUE(TestService.Installed());
}

TEST(ServiceTest, ServiceStart)
{
    if (!IsUserAdmin())
        return;

    if (!TestService.Installed())
        return;

    EXPECT_TRUE(TestService.Start());

    SERVICE_STATUS ss;
    if (TestService.QueryServiceStatus(ss))
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

    if (!TestService.Installed())
        return;

    EXPECT_TRUE(TestService.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP));
}

TEST(ServiceTest, ServicePause)
{
    if (!IsUserAdmin())
        return;

    if (!TestService.Installed())
        return;

    EXPECT_TRUE(TestService.Pause());
}

TEST(ServiceTest, ServiceContinue)
{
    if (!IsUserAdmin())
        return;

    if (!TestService.Installed())
        return;

    EXPECT_TRUE(TestService.Continue());
}

TEST(ServiceTest, ServiceStop)
{
    if (!IsUserAdmin())
        return;

    if (!TestService.Installed())
        return;

    EXPECT_TRUE(TestService.Stop());

    SERVICE_STATUS ss;
    if (TestService.QueryServiceStatus(ss))
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

    if (!TestService.Installed())
        return;

    EXPECT_TRUE(TestService.Uninstall());
    EXPECT_FALSE(TestService.Installed());
}

TEST(ServiceTest, ServiceWInstall)
{
    if (!IsUserAdmin())
        return;

    using namespace Win32Ex::Convert::String;

    std::wstring path =
#ifdef _INC__MINGW_H
        testSvcPath.empty() ? TestServiceW.BinaryPathName() : !testSvcPath;
#else
        TestServiceW.BinaryPathName();
#endif
    path.append(L" " _W(TEST_SVC_NAME) L" W");
    EXPECT_TRUE(TestServiceW.Install(SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, path));
    EXPECT_TRUE(TestServiceW.Installed());
}

TEST(ServiceTest, ServiceWStart)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceW.Installed())
        return;

    EXPECT_TRUE(TestServiceW.Start());

    SERVICE_STATUS ss;
    if (TestServiceW.QueryServiceStatus(ss))
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

    if (!TestServiceW.Installed())
        return;

    EXPECT_TRUE(TestServiceW.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP));
}

TEST(ServiceTest, ServiceWPause)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceW.Installed())
        return;

    EXPECT_TRUE(TestServiceW.Pause());
}

TEST(ServiceTest, ServiceWContinue)
{
    if (!IsUserAdmin())
        return;

    if (!TestService.Installed())
        return;

    EXPECT_TRUE(TestService.Continue());
}

TEST(ServiceTest, ServiceWStop)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceW.Installed())
        return;

    EXPECT_TRUE(TestServiceW.Stop());

    SERVICE_STATUS ss;
    if (TestServiceW.QueryServiceStatus(ss))
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

    if (!TestServiceW.Installed())
        return;

    EXPECT_TRUE(TestServiceW.Uninstall());
    EXPECT_FALSE(TestServiceW.Installed());
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
    EXPECT_TRUE(TestService.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path.c_str()));
    EXPECT_TRUE(TestService.Installed());

    EXPECT_TRUE(Test2Service.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path.c_str()));
    EXPECT_TRUE(Test2Service.Installed());
}

TEST(ServiceTest, SharedServiceStart)
{
    if (!IsUserAdmin())
        return;

    if (TestService.Installed())
    {
        EXPECT_TRUE(TestService.Start());
        SERVICE_STATUS ss;
        if (TestService.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }

    if (Test2Service.Installed())
    {
        EXPECT_TRUE(Test2Service.Start());
        SERVICE_STATUS ss;
        if (Test2Service.QueryServiceStatus(ss))
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

    if (TestService.Installed())
    {
        EXPECT_TRUE(TestService.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP));
    }
    if (Test2Service.Installed())
    {
        EXPECT_TRUE(Test2Service.Control(TEST2_SVC_USER_CONTROL_ACCEPT_STOP));
    }
}

TEST(ServiceTest, SharedServicePause)
{
    if (!IsUserAdmin())
        return;

    if (TestService.Installed())
        EXPECT_TRUE(TestService.Pause());

    if (Test2Service.Installed())
        EXPECT_TRUE(Test2Service.Pause());
}

TEST(ServiceTest, SharedServiceContinue)
{
    if (!IsUserAdmin())
        return;
    if (TestService.Installed())
        EXPECT_TRUE(TestService.Continue());

    if (Test2Service.Installed())
        EXPECT_TRUE(Test2Service.Continue());
}

TEST(ServiceTest, SharedServiceStop)
{
    if (!IsUserAdmin())
        return;

    if (TestService.Installed())
    {
        EXPECT_TRUE(TestService.Stop());
        SERVICE_STATUS ss;
        if (TestService.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
    if (Test2Service.Installed())
    {
        EXPECT_TRUE(Test2Service.Stop());
        SERVICE_STATUS ss;
        if (Test2Service.QueryServiceStatus(ss))
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

    if (TestService.Installed())
    {
        EXPECT_TRUE(TestService.Uninstall());
        EXPECT_FALSE(TestService.Installed());
    }

    if (Test2Service.Installed())
    {
        EXPECT_TRUE(Test2Service.Uninstall());
        EXPECT_FALSE(Test2Service.Installed());
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

    EXPECT_TRUE(TestServiceW.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path));
    EXPECT_TRUE(TestServiceW.Installed());

    EXPECT_TRUE(Test2ServiceW.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path));
    EXPECT_TRUE(Test2ServiceW.Installed());
}

TEST(ServiceTest, SharedServiceWStart)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceW.Installed())
    {
        EXPECT_TRUE(TestServiceW.Start());
        SERVICE_STATUS ss;
        if (TestServiceW.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }

    if (Test2ServiceW.Installed())
    {
        EXPECT_TRUE(Test2ServiceW.Start());
        SERVICE_STATUS ss;
        if (Test2ServiceW.QueryServiceStatus(ss))
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

    if (TestServiceW.Installed())
        EXPECT_TRUE(TestServiceW.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP));

    if (Test2ServiceW.Installed())
        EXPECT_TRUE(Test2ServiceW.Control(TEST2_SVC_USER_CONTROL_ACCEPT_STOP));
}

TEST(ServiceTest, SharedServiceWPause)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceW.Installed())
        EXPECT_TRUE(TestServiceW.Pause());

    if (Test2ServiceW.Installed())
        EXPECT_TRUE(Test2ServiceW.Pause());
}

TEST(ServiceTest, SharedServiceWContinue)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceW.Installed())
        EXPECT_TRUE(TestServiceW.Continue());

    if (Test2ServiceW.Installed())
        EXPECT_TRUE(Test2ServiceW.Continue());
}

TEST(ServiceTest, SharedServiceWStop)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceW.Installed())
    {
        EXPECT_TRUE(TestServiceW.Stop());
        SERVICE_STATUS ss;
        if (TestServiceW.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
    if (Test2ServiceW.Installed())
    {
        EXPECT_TRUE(Test2ServiceW.Stop());
        SERVICE_STATUS ss;
        if (Test2ServiceW.QueryServiceStatus(ss))
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

    if (TestServiceW.Installed())
    {
        EXPECT_TRUE(TestServiceW.Uninstall());
        EXPECT_FALSE(TestServiceW.Installed());
    }

    if (Test2ServiceW.Installed())
    {
        EXPECT_TRUE(Test2ServiceW.Uninstall());
        EXPECT_FALSE(Test2ServiceW.Installed());
    }
}
