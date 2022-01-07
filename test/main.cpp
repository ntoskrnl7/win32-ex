#include <gtest/gtest.h>
#include <string>

#define TWIN32EX_USE_SERVICE_SIMULATE_MODE
#include <Win32Ex/System/Service.hpp>

#include "System/TestService.h"
Win32Ex::System::Service TestService(TEST_SVC_NAME, TEST_SVC_DISPLAY_NAME, TEST_SVC_DESCRIPTION);
typedef Win32Ex::System::Service::Instance<TestService> TestServiceInstance;

Win32Ex::System::Service Test2Service(TEST2_SVC_NAME, TEST2_SVC_DISPLAY_NAME, TEST2_SVC_DESCRIPTION);
typedef Win32Ex::System::Service::Instance<Test2Service> Test2ServiceInstance;

Win32Ex::System::ServiceW TestServiceW(_W(TEST_SVC_NAME), _W(TEST_SVC_DISPLAY_NAME), _W(TEST_SVC_DESCRIPTION));
typedef Win32Ex::System::ServiceW::Instance<TestServiceW> TestServiceInstanceW;

Win32Ex::System::ServiceW Test2ServiceW(_W(TEST2_SVC_NAME), _W(TEST2_SVC_DISPLAY_NAME), _W(TEST2_SVC_DESCRIPTION));
typedef Win32Ex::System::ServiceW::Instance<Test2ServiceW> Test2ServiceInstanceW;

Win32Ex::System::ServiceT<> TestServiceT(TEXT(TEST_SVC_NAME), TEXT(TEST_SVC_DISPLAY_NAME), TEXT(TEST_SVC_DESCRIPTION));
typedef Win32Ex::System::ServiceT<>::Instance<TestServiceT> TestServiceInstanceT;

Win32Ex::System::ServiceT<> Test2ServiceT(TEXT(TEST2_SVC_NAME), TEXT(TEST2_SVC_DISPLAY_NAME),
                                          TEXT(TEST2_SVC_DESCRIPTION));
typedef Win32Ex::System::ServiceT<>::Instance<Test2ServiceT> Test2ServiceInstanceT;

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    if (Win32Ex::System::IsServiceMode())
    {
        if (argc == 2)
        {
#ifdef __cpp_lambdas
            TestService.SetAcceptStop([]() -> bool { return TestService.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP); });

            Test2Service.SetAcceptStop(
                []() -> bool { return Test2Service.Control(TEST2_SVC_USER_CONTROL_ACCEPT_STOP); });
#endif
            if (std::string(argv[1]) == TEST_SVC_NAME)
            {
                TestServiceInstance &svc = TestServiceInstance::Get();
#ifdef __cpp_lambdas
                svc.OnStart([&svc]() {
                       svc.ClearControlsAccepted(SERVICE_ACCEPT_STOP);
                       svc.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
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
                    return EXIT_SUCCESS;

                return GetLastError();
            }
            else if (std::string(argv[1]) == "SharedService")
            {
                TestServiceInstance &svc = TestServiceInstance::Get();
                Test2ServiceInstance &svc2 = Test2ServiceInstance::Get();
#ifdef __cpp_lambdas
                svc.OnStart([&svc]() {
                       svc.ClearControlsAccepted(SERVICE_ACCEPT_STOP);
                       svc.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
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
                        svc2.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
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
                return Win32Ex::System::Service::Run(svc, svc2) ? EXIT_SUCCESS : GetLastError();
            }
        }
        else if (argc == 3)
        {
            if (argv[2][0] == 'W')
            {
#ifdef __cpp_lambdas
                TestServiceW.SetAcceptStop(
                    []() -> bool { return TestServiceW.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP); });

                Test2ServiceW.SetAcceptStop(
                    []() -> bool { return Test2ServiceW.Control(TEST2_SVC_USER_CONTROL_ACCEPT_STOP); });
#endif
                if (std::string(argv[1]) == TEST_SVC_NAME)
                {
                    TestServiceInstanceW &svc = TestServiceInstanceW::Get();
#ifdef __cpp_lambdas
                    svc.OnStart([&svc]() {
                           svc.ClearControlsAccepted(SERVICE_ACCEPT_STOP);
                           svc.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
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
                        return EXIT_SUCCESS;

                    return GetLastError();
                }
                else if (std::string(argv[1]) == "SharedService")
                {
                    TestServiceInstanceW &svc = TestServiceInstanceW::Get();
                    Test2ServiceInstanceW &svc2 = Test2ServiceInstanceW::Get();
#ifdef __cpp_lambdas
                    svc.OnStart([&svc]() {
                           svc.ClearControlsAccepted(SERVICE_ACCEPT_STOP);
                           svc.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
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
                            svc2.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
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
                    return Win32Ex::System::ServiceW::Run<TestServiceInstanceW, Test2ServiceInstanceW>()
                               ? EXIT_SUCCESS
                               : GetLastError();
                }
            }
            else if (argv[2][0] == 'T')
            {
#ifdef __cpp_lambdas
                TestServiceT.SetAcceptStop(
                    []() -> bool { return TestServiceT.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP); });

                Test2ServiceT.SetAcceptStop(
                    []() -> bool { return Test2ServiceT.Control(TEST2_SVC_USER_CONTROL_ACCEPT_STOP); });
#endif
                if (std::string(argv[1]) == TEST_SVC_NAME)
                {
                    TestServiceInstanceT &svc = TestServiceInstanceT::Get();
#ifdef __cpp_lambdas
                    svc.OnStart([&svc]() {
                           svc.ClearControlsAccepted(SERVICE_ACCEPT_STOP);
                           svc.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
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
                        return EXIT_SUCCESS;

                    return GetLastError();
                }
                else if (std::string(argv[1]) == "SharedService")
                {
                    TestServiceInstanceT &svc = TestServiceInstanceT::Get();
                    Test2ServiceInstanceT &svc2 = Test2ServiceInstanceT::Get();
#ifdef __cpp_lambdas
                    svc.OnStart([&svc]() {
                           svc.ClearControlsAccepted(SERVICE_ACCEPT_STOP);
                           svc.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
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
                            svc2.SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
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
                    return Win32Ex::System::ServiceT<>::Run<TestServiceInstanceT, Test2ServiceInstanceT>()
                               ? EXIT_SUCCESS
                               : GetLastError();
                }
            }
        }
    }
    return EXIT_FAILURE;
}
