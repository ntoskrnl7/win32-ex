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

    std::cout << "\n\n-----------------ProfSvc.Dependencies-------------------\n";
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
    std::cout << "\n\n-----------------ProfSvc.DependentServices-------------------\n";
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

TEST(ServiceTest, RequiredPrivileges)
{
    using namespace Win32Ex;
    System::Service service("ProfSvc");
    std::cout << "\n\n-----------------ProfSvc.RequiredPrivileges-------------------\n";
#if defined(__cpp_range_based_for)
    for (auto &privilege : service.RequiredPrivileges().Get({}))
        std::cout << privilege << '\n';
#elif defined(_MSC_VER)
    // clang-format off
    for each (const Win32Ex::String &privilege in service.RequiredPrivileges().Get(std::list<Win32Ex::String>()))
        std::cout << privilege << '\n';
        // clang-format on
#endif
}

const wchar_t *TriggerTypeStringW(DWORD dwTriggerType)
{
    switch (dwTriggerType)
    {
    case SERVICE_TRIGGER_TYPE_DEVICE_INTERFACE_ARRIVAL:
        return L"SERVICE_TRIGGER_TYPE_DEVICE_INTERFACE_ARRIVAL";
    case SERVICE_TRIGGER_TYPE_IP_ADDRESS_AVAILABILITY:
        return L"SERVICE_TRIGGER_TYPE_IP_ADDRESS_AVAILABILITY";
    case SERVICE_TRIGGER_TYPE_DOMAIN_JOIN:
        return L"SERVICE_TRIGGER_TYPE_DOMAIN_JOIN";
    case SERVICE_TRIGGER_TYPE_FIREWALL_PORT_EVENT:
        return L"SERVICE_TRIGGER_TYPE_FIREWALL_PORT_EVENT";
    case SERVICE_TRIGGER_TYPE_GROUP_POLICY:
        return L"SERVICE_TRIGGER_TYPE_GROUP_POLICY";
    case SERVICE_TRIGGER_TYPE_NETWORK_ENDPOINT:
        return L"SERVICE_TRIGGER_TYPE_NETWORK_ENDPOINT";
    case SERVICE_TRIGGER_TYPE_CUSTOM_SYSTEM_STATE_CHANGE:
        return L"SERVICE_TRIGGER_TYPE_CUSTOM_SYSTEM_STATE_CHANGE";
    case SERVICE_TRIGGER_TYPE_CUSTOM:
        return L"SERVICE_TRIGGER_TYPE_CUSTOM";
    case SERVICE_TRIGGER_TYPE_AGGREGATE:
        return L"SERVICE_TRIGGER_TYPE_AGGREGATE";
    default:
        return L"SERVICE_TRIGGER_TYPE_UNKNOWN";
    }
}

const wchar_t *ActionStringW(DWORD dwAction)
{
    const char *action = NULL;
    switch (dwAction)
    {
    case SERVICE_TRIGGER_ACTION_SERVICE_START:
        return L"SERVICE_TRIGGER_ACTION_SERVICE_START";
    case SERVICE_TRIGGER_ACTION_SERVICE_STOP:
        return L"SERVICE_TRIGGER_ACTION_SERVICE_STOP";
    default:
        return L"SERVICE_TRIGGER_ACTION_UNKNOWN";
    }
}

const char *TriggerTypeString(DWORD dwTriggerType)
{
    switch (dwTriggerType)
    {
    case SERVICE_TRIGGER_TYPE_DEVICE_INTERFACE_ARRIVAL:
        return "SERVICE_TRIGGER_TYPE_DEVICE_INTERFACE_ARRIVAL";
    case SERVICE_TRIGGER_TYPE_IP_ADDRESS_AVAILABILITY:
        return "SERVICE_TRIGGER_TYPE_IP_ADDRESS_AVAILABILITY";
    case SERVICE_TRIGGER_TYPE_DOMAIN_JOIN:
        return "SERVICE_TRIGGER_TYPE_DOMAIN_JOIN";
    case SERVICE_TRIGGER_TYPE_FIREWALL_PORT_EVENT:
        return "SERVICE_TRIGGER_TYPE_FIREWALL_PORT_EVENT";
    case SERVICE_TRIGGER_TYPE_GROUP_POLICY:
        return "SERVICE_TRIGGER_TYPE_GROUP_POLICY";
    case SERVICE_TRIGGER_TYPE_NETWORK_ENDPOINT:
        return "SERVICE_TRIGGER_TYPE_NETWORK_ENDPOINT";
    case SERVICE_TRIGGER_TYPE_CUSTOM_SYSTEM_STATE_CHANGE:
        return "SERVICE_TRIGGER_TYPE_CUSTOM_SYSTEM_STATE_CHANGE";
    case SERVICE_TRIGGER_TYPE_CUSTOM:
        return "SERVICE_TRIGGER_TYPE_CUSTOM";
    case SERVICE_TRIGGER_TYPE_AGGREGATE:
        return "SERVICE_TRIGGER_TYPE_AGGREGATE";
    default:
        return "SERVICE_TRIGGER_TYPE_UNKNOWN";
    }
}

const char *ActionString(DWORD dwAction)
{
    const char *action = NULL;
    switch (dwAction)
    {
    case SERVICE_TRIGGER_ACTION_SERVICE_START:
        return "SERVICE_TRIGGER_ACTION_SERVICE_START";
    case SERVICE_TRIGGER_ACTION_SERVICE_STOP:
        return "SERVICE_TRIGGER_ACTION_SERVICE_STOP";
    default:
        return "SERVICE_TRIGGER_ACTION_UNKNOWN";
    }
}

template <typename _CharType> const _CharType *TriggerTypeStringT(DWORD dwTriggerType);

template <> const char *TriggerTypeStringT<char>(DWORD dwTriggerType)
{
    return TriggerTypeString(dwTriggerType);
}
template <> const wchar_t *TriggerTypeStringT<wchar_t>(DWORD dwTriggerType)
{
    return TriggerTypeStringW(dwTriggerType);
}

template <typename _CharType> const _CharType *ActionStringT(DWORD dwTriggerType);

template <> const char *ActionStringT<char>(DWORD dwTriggerType)
{
    return ActionString(dwTriggerType);
}
template <> const wchar_t *ActionStringT<wchar_t>(DWORD dwTriggerType)
{
    return ActionStringW(dwTriggerType);
}

TEST(ServiceTest, ServiceTAll)
{
    // clang-format off
#if defined(UNICODE)
    std::wostream &tcout = std::wcout;
#else
    std::ostream &tcout = std::cout;
#endif
#if defined(__cpp_range_based_for)
    for (auto service : Win32Ex::System::ServiceT<>::All())
#else
    for each (Win32Ex::System::ServiceT<> service in Win32Ex::System::ServiceT<>::All())
#endif
    {
        tcout << TEXT("Name : ") << service.Name() << TEXT('\n');
        tcout << TEXT("BinPath : ") << service.BinaryPathName() << TEXT('\n');
        tcout << TEXT("Description : ") << service.Description() << TEXT('\n');

        Win32Ex::Result<SERVICE_PRESHUTDOWN_INFO> preshutdownInfo = service.PreshutdownTimeout();
        if (preshutdownInfo.IsOk())
            std::cout << "Preshutdown : " << preshutdownInfo.Get().dwPreshutdownTimeout << "ms\n";
        
        Win32Ex::Result<Win32Ex::SharedPtr<SERVICE_FAILURE_ACTIONS>> failureAction = service.FailureActions();
        if (failureAction.IsOk() && failureAction.Ref())
        {
            SERVICE_FAILURE_ACTIONS &actions = *failureAction.Ref();
            if (actions.lpRebootMsg)
                tcout << TEXT("Failrue RebootMsg: ") << actions.lpRebootMsg << TEXT('\n');
            if (actions.lpRebootMsg)
                tcout << TEXT("Failrue Command: ") << actions.lpCommand << TEXT('\n');
            if (actions.cActions)
                std::cout << "Failrue Actions : " << actions.cActions << "\n";
        }
#if defined(SERVICE_CONFIG_TRIGGER_INFO)
        Win32Ex::Result<Win32Ex::SharedPtr<SERVICE_TRIGGER_INFO>> result = service.Trigger();
        if (result.IsOk() && result.Ref())
        {
            {
                SERVICE_TRIGGER_INFO &triggers = *result.Ref();
                if (triggers.cTriggers)
                {
                    tcout << TEXT("Triggers\n");
                    for (DWORD i = 0; i < triggers.cTriggers; i++)
                        tcout << TEXT("\t- Type : ") << TriggerTypeStringT<TCHAR>(triggers.pTriggers[i].dwTriggerType)
                              << TEXT("\t Action : ") << ActionStringT<TCHAR>(triggers.pTriggers[i].dwAction) << TEXT('\n');
                }
            }
        }
#endif
#if defined(__cpp_initializer_lists)
        auto privileges = service.RequiredPrivileges().Get({});
#else
        std::list<Win32Ex::StringT> privileges = service.RequiredPrivileges().Get(std::list<Win32Ex::StringT>());
#endif
        if (!privileges.empty())
        {
            tcout << TEXT("Privileges\n");
#if defined(__cpp_range_based_for)
            for (auto privilege : privileges)
#else
            for each (const Win32Ex::StringT &privilege in privileges)
#endif
                tcout << TEXT("\t- ") << privilege << TEXT('\n');
        }
    }
    // clang-format on
}

TEST(ServiceTest, ServiceAll)
{
    // clang-format off
#if defined(__cpp_range_based_for)
    for (auto service : Win32Ex::System::Service::All())
#else
    for each (Win32Ex::System::Service service in Win32Ex::System::Service::All())
#endif
    {
        std::cout << "Name : " << service.Name() << '\n';
        std::cout << "BinPath : " << service.BinaryPathName() << '\n';
        std::cout << "Description : " << service.Description() << '\n';

        Win32Ex::Result<SERVICE_PRESHUTDOWN_INFO> preshutdownInfo = service.PreshutdownTimeout();
        if (preshutdownInfo.IsOk())
            std::cout << "Preshutdown : " << preshutdownInfo.Get().dwPreshutdownTimeout << "ms\n";

        Win32Ex::Result<Win32Ex::SharedPtr<SERVICE_FAILURE_ACTIONSA>> failureAction = service.FailureActions();
        if (failureAction.IsOk() && failureAction.Ref())
        {
            SERVICE_FAILURE_ACTIONSA &actions = *failureAction.Ref();
            if (actions.lpRebootMsg)
                std::cout << "Failrue RebootMsg: " << actions.lpRebootMsg << "\n";
            if (actions.lpRebootMsg)
                std::cout << "Failrue Command: " << actions.lpCommand << "\n";
            if (actions.cActions)
                std::cout << "Failrue Actions : " << actions.cActions << "\n";
        }
#if defined(SERVICE_CONFIG_TRIGGER_INFO)
        Win32Ex::Result<Win32Ex::SharedPtr<SERVICE_TRIGGER_INFO>> result = service.Trigger();
        if (result.IsOk() && result.Ref())
        {
            SERVICE_TRIGGER_INFO &triggers = *result.Ref();
            if (triggers.cTriggers)
            {
                std::cout << "Triggers\n";
                for (DWORD i = 0; i < triggers.cTriggers; i++)
                {
                    std::cout << "\t- Type : " << TriggerTypeString(triggers.pTriggers[i].dwTriggerType)
                              << "\t Action : " << ActionString(triggers.pTriggers[i].dwAction) << '\n';
                }
            }
        }
#endif
#if defined(__cpp_initializer_lists)
        auto privileges = service.RequiredPrivileges().Get({});
#else
        std::list<Win32Ex::String> privileges = service.RequiredPrivileges().Get(std::list<Win32Ex::String>());
#endif
        if (!privileges.empty())
        {
            std::cout << "Privileges\n";
#if defined(__cpp_range_based_for)
            for (auto privilege : privileges)
#else
            for each (const Win32Ex::String &privilege in privileges)
#endif
                std::cout << "\t- " << privilege << '\n';
        }
    }
    // clang-format on
}

TEST(ServiceTest, ServiceWAll)
{
    // clang-format off
#if defined(__cpp_range_based_for)
    for (auto service : Win32Ex::System::ServiceW::All())
#else
    for each (Win32Ex::System::ServiceW service in Win32Ex::System::ServiceW::All())
#endif
    {
        std::wcout << L"Name : " << service.Name() << L'\n';
        std::wcout << L"BinPath : " << service.BinaryPathName() << L'\n';
        std::wcout << L"Description : " << service.Description() << L'\n';

        Win32Ex::Result<SERVICE_PRESHUTDOWN_INFO> preshutdownInfo = service.PreshutdownTimeout();
        if (preshutdownInfo.IsOk())
            std::wcout << L"Preshutdown : " << preshutdownInfo.Get().dwPreshutdownTimeout << L"ms\n";

        Win32Ex::Result<Win32Ex::SharedPtr<SERVICE_FAILURE_ACTIONSW>> failureAction = service.FailureActions();
        if (failureAction.IsOk() && failureAction.Ref())
        {
            SERVICE_FAILURE_ACTIONSW &actions = *failureAction.Ref();
            if (actions.lpRebootMsg)
                std::wcout << L"Failrue RebootMsg: " << actions.lpRebootMsg << L"\n";
            if (actions.lpRebootMsg)
                std::wcout << L"Failrue Command: " << actions.lpCommand << L"\n";
            if (actions.cActions)
                std::wcout << L"Failrue Actions : " << actions.cActions << L"\n";
        }
#if defined(SERVICE_CONFIG_TRIGGER_INFO)
        Win32Ex::Result<Win32Ex::SharedPtr<SERVICE_TRIGGER_INFO>> result = service.Trigger();
        if (result.IsOk() && result.Ref())
        {
            SERVICE_TRIGGER_INFO &triggers = *result.Ref();
            if (triggers.cTriggers)
            {
                std::wcout << L"Triggers\n";
                for (DWORD i = 0; i < triggers.cTriggers; i++)
                {
                    std::wcout << L"\t- Type : " << TriggerTypeStringW(triggers.pTriggers[i].dwTriggerType)
                               << L"\t Action : " << ActionStringW(triggers.pTriggers[i].dwAction) << L'\n';
                }
            }
        }
#endif
#if defined(__cpp_initializer_lists)
        auto privileges = service.RequiredPrivileges().Get({});
#else
        std::list<Win32Ex::StringW> privileges = service.RequiredPrivileges().Get(std::list<Win32Ex::StringW>());
#endif
        if (!privileges.empty())
        {
            std::wcout << L"Privileges\n";
#if defined(__cpp_range_based_for)
            for (auto privilege : privileges)
#else
            for each (const Win32Ex::StringW &privilege in privileges)
#endif
                std::wcout << L"\t- " << privilege << L'\n';
        }
    }
    // clang-format on
}

#ifdef SERVICE_CONFIG_TRIGGER_INFO
TEST(ServiceTest, ServiceTrigger)
{
    using namespace Win32Ex;
    System::Service service("DeviceInstall");
    Result<SharedPtr<SERVICE_TRIGGER_INFO>> result = service.Trigger();
    if (result.IsOk() && result.Ref())
    {
        SERVICE_TRIGGER_INFO &triggers = *result.Ref();
        if (triggers.cTriggers)
        {
            std::cout << "Triggers\n";
            for (DWORD i = 0; i < triggers.cTriggers; i++)
            {
                std::cout << "\t- Type : " << TriggerTypeString(triggers.pTriggers[i].dwTriggerType)
                          << "\t Action : " << ActionString(triggers.pTriggers[i].dwAction) << '\n';
            }
        }
    }
}
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
    EXPECT_TRUE(TestServiceT.Installed());
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
    EXPECT_TRUE(TestServiceT.Installed());

    EXPECT_TRUE(Test2ServiceT.Install(SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START, path));
    EXPECT_TRUE(Test2ServiceT.Installed());
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
