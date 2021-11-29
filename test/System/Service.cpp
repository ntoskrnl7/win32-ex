#include <System/Process.hpp>

#define WIN32EX_USE_SERVICE_SIMULATE_CONSOLE_MODE
#include <System/Service.hpp>
#include <gtest/gtest.h>

#include "TestService.h"
extern Win32Ex::System::ServiceConfig TestServiceConfig;
typedef Win32Ex::System::Service<TestServiceConfig> TestService;

extern Win32Ex::System::ServiceConfig Test2ServiceConfig;
typedef Win32Ex::System::Service<Test2ServiceConfig> Test2Service;

#ifndef WIN32EX_USE_SERVICE_SIMULATE_CONSOLE_MODE
TEST(ServiceTest, InvalidServiceRun)
{
    TestService &svc = TestService::Instance();
    EXPECT_FALSE(svc.Run());
}
#endif

TEST(ServiceTest, ServiceInstall)
{
    if (!IsUserAdmin())
    {
        return;
    }

    std::string path = TestServiceConfig.GetBinaryPathName();
    path.append(" " TEST_SVC_NAME);
    EXPECT_TRUE(TestServiceConfig.Install(SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, path.c_str()));
    EXPECT_TRUE(TestServiceConfig.Installed());
}

TEST(ServiceTest, ServiceStart)
{
    if (!IsUserAdmin())
    {
        return;
    }

    if (!TestServiceConfig.Installed())
    {
        return;
    }

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
    {
        return;
    }

    if (!TestServiceConfig.Installed())
    {
        return;
    }

    EXPECT_TRUE(TestServiceConfig.Control(TEST_SVC_USER_CONTROL));
}

TEST(ServiceTest, ServicePause)
{
    if (!IsUserAdmin())
    {
        return;
    }

    if (!TestServiceConfig.Installed())
    {
        return;
    }

    EXPECT_TRUE(TestServiceConfig.Pause());
}

TEST(ServiceTest, ServiceContinue)
{
    if (!IsUserAdmin())
    {
        return;
    }

    if (!TestServiceConfig.Installed())
    {
        return;
    }

    EXPECT_TRUE(TestServiceConfig.Continue());
}

TEST(ServiceTest, ServiceStop)
{
    if (!IsUserAdmin())
    {
        return;
    }

    if (!TestServiceConfig.Installed())
    {
        return;
    }

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
    {
        return;
    }

    if (!TestServiceConfig.Installed())
    {
        return;
    }

    EXPECT_TRUE(TestServiceConfig.Uninstall());
    // EXPECT_FALSE(TestServiceConfig.Installed());
}

TEST(ServiceTest, Etc)
{
    Win32Ex::System::IsServiceMode();
}

TEST(ServiceTest, SharedServiceInstall)
{
    if (!IsUserAdmin())
    {
        return;
    }

    std::string path = Win32Ex::ThisProcess::GetExecutablePath();
    path.append(" SharedService");
    EXPECT_TRUE(TestServiceConfig.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path.c_str()));
    EXPECT_TRUE(TestServiceConfig.Installed());

    EXPECT_TRUE(Test2ServiceConfig.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path.c_str()));
    EXPECT_TRUE(Test2ServiceConfig.Installed());
}

TEST(ServiceTest, SharedServiceStart)
{
    if (!IsUserAdmin())
    {
        return;
    }

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
    {
        return;
    }
    if (TestServiceConfig.Installed())
    {
        EXPECT_TRUE(TestServiceConfig.Control(TEST_SVC_USER_CONTROL));
    }
    if (Test2ServiceConfig.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfig.Control(TEST_SVC_USER_CONTROL));
    }
}

TEST(ServiceTest, SharedServicePause)
{
    if (!IsUserAdmin())
    {
        return;
    }

    if (TestServiceConfig.Installed())
    {
        EXPECT_TRUE(TestServiceConfig.Pause());
    }
    if (Test2ServiceConfig.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfig.Pause());
    }
}

TEST(ServiceTest, SharedServiceContinue)
{
    if (!IsUserAdmin())
    {
        return;
    }

    if (TestServiceConfig.Installed())
    {
        EXPECT_TRUE(TestServiceConfig.Continue());
    }
    if (Test2ServiceConfig.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfig.Continue());
    }
}

TEST(ServiceTest, SharedServiceStop)
{
    if (!IsUserAdmin())
    {
        return;
    }

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
    {
        return;
    }

    if (TestServiceConfig.Installed())
    {
        EXPECT_TRUE(TestServiceConfig.Uninstall());
        // EXPECT_FALSE(TestServiceConfig.Installed());
    }

    if (Test2ServiceConfig.Installed())
    {
        EXPECT_TRUE(Test2ServiceConfig.Uninstall());
        // EXPECT_FALSE(Test2ServiceConfig.Installed());
    }
}
