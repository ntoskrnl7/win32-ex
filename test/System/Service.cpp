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

TEST(ServiceTest, DependentServices)
{
    using namespace Win32Ex;
    System::Service service("ProfSvc");

    std::cout << "\n\n-----------------Dependencies-------------------\n";
#if defined(__cpp_range_based_for)
    for (auto &dep : service.Dependencies())
    {
        std::cout << dep.Name() << "\n\t" << dep.DisplayName() << "\n\t" << dep.BinaryPathName() << '\n';
        for (auto &dep2 : dep.Dependencies())
            std::cout << "\t\t" << dep2.Name() << "\n\t\t\t" << dep2.DisplayName() << "\n\t\t\t"
                      << dep2.BinaryPathName() << '\n';
    }
#elif defined(_MSC_VER)
    // clang-format off
    for each (const System::Service &dep in service.Dependencies())
    {
        std::cout << dep.Name() << "\n\t" << dep.DisplayName() << "\n\t" << dep.BinaryPathName() << '\n';
        for each (const System::Service &dep2 in dep.Dependencies())
            std::cout << "\t\t" << dep2.Name() << "\n\t\t\t" << dep2.DisplayName() << "\n\t\t\t"
                      << dep2.BinaryPathName() << '\n';
    }
        // clang-format on
#endif
    std::cout << "\n\n-----------------DependentServices-------------------\n";
#if defined(__cpp_range_based_for)
    for (auto &dep : service.DependentServices().Get({}))
    {
        std::cout << dep.Name() << "\n\t" << dep.DisplayName() << "\n\t" << dep.BinaryPathName() << '\n';
        for (auto &dep2 : dep.DependentServices().Get({}))
            std::cout << "\t\t" << dep2.Name() << "\n\t\t\t" << dep2.DisplayName() << "\n\t\t\t"
                      << dep2.BinaryPathName() << '\n';
    }
#elif defined(_MSC_VER)
    // clang-format off
    for each (const System::Service &dep in service.DependentServices().Get(std::list<System::Service>()))
    {
            std::cout << dep.Name() << "\n\t" << dep.DisplayName() << "\n\t" << dep.BinaryPathName() << '\n';
        for each (const System::Service &dep2 in dep.DependentServices().Get(std::list<System::Service>()))
            std::cout << "\t\t" << dep2.Name() << "\n\t\t\t" << dep2.DisplayName() << "\n\t\t\t"
                      << dep2.BinaryPathName() << '\n';
    }
#endif
    // clang-format on
}

#include "TestService.h"
extern Win32Ex::System::Service TestService;
typedef Win32Ex::System::Service::Instance<TestService> TestServiceInstance;

extern Win32Ex::System::Service Test2Service;
typedef Win32Ex::System::Service::Instance<Test2Service> Test2ServiceInstance;

extern Win32Ex::System::ServiceW TestServiceW;
typedef Win32Ex::System::ServiceW::Instance<TestServiceW> TestServiceInstanceW;

extern Win32Ex::System::ServiceW Test2ServiceW;
typedef Win32Ex::System::ServiceW::Instance<Test2ServiceW> Test2ServiceInstanceW;

extern Win32Ex::System::ServiceT<> TestServiceT;
typedef Win32Ex::System::ServiceT<>::Instance<TestServiceT> TestServiceInstanceT;

extern Win32Ex::System::ServiceT<> Test2ServiceT;
typedef Win32Ex::System::ServiceT<>::Instance<Test2ServiceT> Test2ServiceInstanceT;

#ifndef TWIN32EX_USE_SERVICE_SIMULATE_MODE
TEST(ServiceTest, InvalidServiceRun)
{
    EXPECT_FALSE(TestServiceInstance::Get().Run());
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
    EXPECT_TRUE(TestServiceInstance::Get().OnStart(OnTestSvcStart).Run());
}

TEST(ServiceTest, SimulateModeW)
{
    EXPECT_TRUE(TestServiceInstanceW::Get().OnStart(OnTest2SvcStart).Run());
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
    // EXPECT_TRUE(TestService.Installed());
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
    // EXPECT_FALSE(TestService.Installed());
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
    // EXPECT_TRUE(TestServiceW.Installed());
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
    // EXPECT_FALSE(TestServiceW.Installed());
}

TEST(ServiceTest, ServiceTInstall)
{
    if (!IsUserAdmin())
        return;

    using namespace Win32Ex::Convert::String;

    Win32Ex::StringT path =
#ifdef _INC__MINGW_H
        testSvcPath.empty() ? TestServiceT.BinaryPathName()
#ifdef UNICODE
                            : !testSvcPath;
#else
                            : testSvcPath;
#endif
#else
        TestServiceT.BinaryPathName();
#endif
    path.append(TEXT(" ") TEXT(TEST_SVC_NAME) TEXT(" W"));
    EXPECT_TRUE(TestServiceT.Install(SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, path));
    // EXPECT_TRUE(TestServiceT.Installed());
}

TEST(ServiceTest, ServiceTStart)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceT.Installed())
        return;

    EXPECT_TRUE(TestServiceT.Start());

    SERVICE_STATUS ss;
    if (TestServiceT.QueryServiceStatus(ss))
    {
        EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
        EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
    }
}

TEST(ServiceTest, ServiceTControl)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceT.Installed())
        return;

    EXPECT_TRUE(TestServiceT.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP));
}

TEST(ServiceTest, ServiceTPause)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceT.Installed())
        return;

    EXPECT_TRUE(TestServiceT.Pause());
}

TEST(ServiceTest, ServiceTContinue)
{
    if (!IsUserAdmin())
        return;

    if (!TestService.Installed())
        return;

    EXPECT_TRUE(TestService.Continue());
}

TEST(ServiceTest, ServiceTStop)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceT.Installed())
        return;

    EXPECT_TRUE(TestServiceT.Stop());

    SERVICE_STATUS ss;
    if (TestServiceT.QueryServiceStatus(ss))
    {
        EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
        EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
    }
}

TEST(ServiceTest, ServiceTUninstall)
{
    if (!IsUserAdmin())
        return;

    if (!TestServiceT.Installed())
        return;

    EXPECT_TRUE(TestServiceT.Uninstall());
    // EXPECT_FALSE(TestServiceT.Installed());
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
    // EXPECT_TRUE(TestService.Installed());

    EXPECT_TRUE(Test2Service.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path.c_str()));
    // EXPECT_TRUE(Test2Service.Installed());
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
        // EXPECT_FALSE(TestService.Installed());
    }

    if (Test2Service.Installed())
    {
        EXPECT_TRUE(Test2Service.Uninstall());
        // EXPECT_FALSE(Test2Service.Installed());
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
    // EXPECT_TRUE(TestServiceW.Installed());

    EXPECT_TRUE(Test2ServiceW.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path));
    // EXPECT_TRUE(Test2ServiceW.Installed());
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
        // EXPECT_FALSE(TestServiceW.Installed());
    }

    if (Test2ServiceW.Installed())
    {
        EXPECT_TRUE(Test2ServiceW.Uninstall());
        // EXPECT_FALSE(Test2ServiceW.Installed());
    }
}

TEST(ServiceTest, SharedServiceTInstall)
{
    if (!IsUserAdmin())
        return;

    using namespace Win32Ex::Convert::String;

    Win32Ex::StringT path =
#ifdef _INC__MINGW_H
        testSvcPath.empty() ? Win32Ex::ThisProcess::ExecutablePathT()
#ifdef UNICODE
                            : !testSvcPath;
#else
                            : testSvcPath;
#endif
#else
#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
        Win32Ex::ThisProcess::ExecutablePathT();
#else
        Win32Ex::ThisProcess::ExecutablePathT<Win32Ex::StringT>();
#endif
#endif

    path.append(TEXT(" SharedService T"));

    EXPECT_TRUE(TestServiceT.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path));
    // EXPECT_TRUE(TestServiceT.Installed());

    EXPECT_TRUE(Test2ServiceT.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path));
    // EXPECT_TRUE(Test2ServiceT.Installed());
}

TEST(ServiceTest, SharedServiceTStart)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceT.Installed())
    {
        EXPECT_TRUE(TestServiceT.Start());
        SERVICE_STATUS ss;
        if (TestServiceT.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }

    if (Test2ServiceT.Installed())
    {
        EXPECT_TRUE(Test2ServiceT.Start());
        SERVICE_STATUS ss;
        if (Test2ServiceT.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOPPED);
            EXPECT_NE(ss.dwCurrentState, SERVICE_STOP_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
}

TEST(ServiceTest, SharedServiceTControl)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceT.Installed())
        EXPECT_TRUE(TestServiceT.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP));

    if (Test2ServiceT.Installed())
        EXPECT_TRUE(Test2ServiceT.Control(TEST2_SVC_USER_CONTROL_ACCEPT_STOP));
}

TEST(ServiceTest, SharedServiceTPause)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceT.Installed())
        EXPECT_TRUE(TestServiceT.Pause());

    if (Test2ServiceT.Installed())
        EXPECT_TRUE(Test2ServiceT.Pause());
}

TEST(ServiceTest, SharedServiceTContinue)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceT.Installed())
        EXPECT_TRUE(TestServiceT.Continue());

    if (Test2ServiceT.Installed())
        EXPECT_TRUE(Test2ServiceT.Continue());
}

TEST(ServiceTest, SharedServiceTStop)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceT.Installed())
    {
        EXPECT_TRUE(TestServiceT.Stop());
        SERVICE_STATUS ss;
        if (TestServiceT.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
    if (Test2ServiceT.Installed())
    {
        EXPECT_TRUE(Test2ServiceT.Stop());
        SERVICE_STATUS ss;
        if (Test2ServiceT.QueryServiceStatus(ss))
        {
            EXPECT_NE(ss.dwCurrentState, SERVICE_RUNNING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_START_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_CONTINUE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSE_PENDING);
            EXPECT_NE(ss.dwCurrentState, SERVICE_PAUSED);
        }
    }
}

TEST(ServiceTest, SharedServiceTUninstall)
{
    if (!IsUserAdmin())
        return;

    if (TestServiceT.Installed())
    {
        EXPECT_TRUE(TestServiceT.Uninstall());
        // EXPECT_FALSE(TestServiceT.Installed());
    }

    if (Test2ServiceT.Installed())
    {
        EXPECT_TRUE(Test2ServiceT.Uninstall());
        // EXPECT_FALSE(Test2ServiceT.Installed());
    }
}
