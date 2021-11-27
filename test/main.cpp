#include <gtest/gtest.h>

#define WIN32EX_USE_SERVICE_SIMULATE_CONSOLE_MODE
#include <System/Service.hpp>

#include "System/TestService.h"
Win32Ex::System::ServiceConfig TestServiceConfig(TEST_SVC_NAME, TEST_SVC_DISPLAY_NAME, TEST_SVC_DESCRIPTION);
typedef Win32Ex::System::Service<TestServiceConfig> TestService;

Win32Ex::System::ServiceConfig Test2ServiceConfig(TEST2_SVC_NAME, TEST2_SVC_DISPLAY_NAME, TEST2_SVC_DESCRIPTION);
typedef Win32Ex::System::Service<Test2ServiceConfig> Test2Service;

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        if (_stricmp(argv[1], TEST_SVC_NAME) == 0)
        {
            TestService &svc = TestService::Instance();
#ifdef __cpp_lambdas
            svc.OnStart((TestService::StartCallback)[](){
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
                .On(TEST_SVC_USER_CONTROL, (TestService::OtherCallback)[](){
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
        else if (_stricmp(argv[1], "SharedService") == 0)
        {
            TestService &svc = TestService::Instance();
            Test2Service &svc2 = Test2Service::Instance();
#ifdef __cpp_lambdas
            svc.OnStart((TestService::StartCallback)[](){
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
                .On(TEST_SVC_USER_CONTROL, (TestService::OtherCallback)[](){
                                               // TODO
                                           });

            svc2.OnStart((Test2Service::StartCallback)[](){
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
                .On(TEST2_SVC_USER_CONTROL, (Test2Service::OtherCallback)[](){
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