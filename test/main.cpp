#include <gtest/gtest.h>
#include <string>

#define WIN32EX_USE_SERVICE_SIMULATE_CONSOLE_MODE
#include <Win32Ex/System/Service.hpp>

#include "System/TestService.h"
Win32Ex::System::ServiceConfig TestServiceConfig(TEST_SVC_NAME, TEST_SVC_DISPLAY_NAME, TEST_SVC_DESCRIPTION);
typedef Win32Ex::System::Service<TestServiceConfig> TestService;

Win32Ex::System::ServiceConfig Test2ServiceConfig(TEST2_SVC_NAME, TEST2_SVC_DISPLAY_NAME, TEST2_SVC_DESCRIPTION);
typedef Win32Ex::System::Service<Test2ServiceConfig> Test2Service;

int main(int argc, char *argv[])
{
#ifdef __cpp_lambdas
    TestServiceConfig.SetAcceptStop([]() -> bool {
        SC_HANDLE serviceHandle = TestServiceConfig.GetServiceHandle(SERVICE_USER_DEFINED_CONTROL);
        if (serviceHandle == NULL)
            return false;
        SERVICE_STATUS ss = {0};
        BOOL result = ControlService(serviceHandle, TEST_SVC_USER_CONTROL_ACCEPT_STOP, &ss);
        CloseServiceHandle(serviceHandle);
        return (result == TRUE);
    });

    Test2ServiceConfig.SetAcceptStop([]() -> bool {
        SC_HANDLE serviceHandle = Test2ServiceConfig.GetServiceHandle(SERVICE_USER_DEFINED_CONTROL);
        if (serviceHandle == NULL)
            return false;
        SERVICE_STATUS ss = {0};
        BOOL result = ControlService(serviceHandle, TEST2_SVC_USER_CONTROL_ACCEPT_STOP, &ss);
        CloseServiceHandle(serviceHandle);
        return (result == TRUE);
    });
#endif
    if (argc == 2)
    {
        if (std::string(argv[1]) == TEST_SVC_NAME)
        {
            TestService &svc = TestService::Instance();
#ifdef __cpp_lambdas
            svc.OnStart([&svc]() {
                   svc.ClearControlsAccepted(SERVICE_ACCEPT_STOP);
                   // TODO
               })
                .OnStop([]() {
                    // TODO
                })
                .OnPause([]() {
                    // TODO
                })
                .OnContinue([]() {
                    // TODO
                })
                .On(TEST_SVC_USER_CONTROL_ACCEPT_STOP, [&svc]() {
                    svc.SetControlsAccepted(SERVICE_ACCEPT_STOP);
                    // TODO
                });
#else
            svc.SetControlsAccepted(SERVICE_ACCEPT_STOP);
            svc.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
#endif
            if (svc.Run())
            {
                return EXIT_SUCCESS;
            }
            return GetLastError();
        }
        else if (std::string(argv[1]) == "SharedService")
        {
            TestService &svc = TestService::Instance();
            Test2Service &svc2 = Test2Service::Instance();
#ifdef __cpp_lambdas
            svc.OnStart([&svc]() {
                   svc.ClearControlsAccepted(SERVICE_ACCEPT_STOP);
                   // TODO
               })
                .OnStop([]() {
                    // TODO
                })
                .OnPause([]() {
                    // TODO
                })
                .OnContinue([]() {
                    // TODO
                })
                .OnError([](DWORD ErrorCode, PCSTR Message) {
                    // TODO
                })
                .On(TEST_SVC_USER_CONTROL_ACCEPT_STOP, [&svc]() {
                    svc.SetControlsAccepted(SERVICE_ACCEPT_STOP);
                    // TODO
                });

            svc2.OnStart([&svc2]() {
                    svc2.ClearControlsAccepted(SERVICE_ACCEPT_STOP);
                    // TODO
                })
                .OnStop([]() {
                    // TODO
                })
                .OnPause([]() {
                    // TODO
                })
                .OnContinue([]() {
                    // TODO
                })
                .OnError([](DWORD ErrorCode, PCSTR Message) {
                    // TODO
                })
                .On(TEST2_SVC_USER_CONTROL_ACCEPT_STOP, [&svc2]() {
                    svc2.SetControlsAccepted(SERVICE_ACCEPT_STOP);
                    // TODO
                });
#else
            svc.SetControlsAccepted(SERVICE_ACCEPT_STOP);
            svc.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
            svc2.SetControlsAccepted(SERVICE_ACCEPT_STOP);
            svc2.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
#endif
            if (Win32Ex::System::Services::Run(svc, svc2))
            {
                return EXIT_SUCCESS;
            }
            return GetLastError();
        }
    }
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}