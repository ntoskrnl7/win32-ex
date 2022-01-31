# Services

- **Link :** <https://docs.microsoft.com/windows/win32/services/services>
- **Headers :** Win32Ex/System/Service.hpp

## Contents

- [Services](#services)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Classes](#classes)
      - [Service](#service)
      - [ServiceW](#servicew)
      - [ServiceT\<class StringType = Win32Ex::StringT\>](#servicetclass-stringtype--win32exstringt)
      - [Service::Instance\<Service\>](#serviceinstanceservice)
      - [ServiceW::Instance\<ServiceW\>](#servicewinstanceservicew)
      - [ServiceT::Instance\<ServiceT\>](#servicetinstanceservicet)
    - [Examples](#examples)

## Reference

### Classes

#### Service

- [Example](#examples)

#### ServiceW

#### ServiceT\<class StringType = Win32Ex::StringT\>

#### Service::Instance\<Service\>

- **Singleton**
- [Example](#examples)

#### ServiceW::Instance\<ServiceW\>

- **Singleton**

#### ServiceT::Instance\<ServiceT\>

- **Singleton**

### Examples

- Enumerate the dependencies and dependent services of User Profile Service (ProfSvc).

  ```C++
  #include <Win32Ex/System/Service.hpp>

  Win32Ex::System::Service service("ProfSvc");

  std::cout << "\n\n-----------------Dependencies-------------------\n";
  for (auto &dep : service.Dependencies())
  {
      std::cout << dep.Name() << '\n';
      for (auto &dep2 : dep.Dependencies())
        std::cout << "\t" << dep2.Name() << '\n';
  }

  std::cout << "\n\n-----------------DependentServices-------------------\n";
  for (auto &dep : service.DependentServices().Get({}))
  {
      std::cout << dep.Name() << '\n';
      for (auto &dep2 : dep.DependentServices().Get({}))
        std::cout << "\t" << dep2.Name() << '\n';
  }
  ```

- Simple service
  - Control Process

    ```C++
    #include <Win32Ex/System/Service.hpp>

    Win32Ex::System::Service SimpleService("SimpleSvc");

    int main()
    {
      SimpleService.Install();
      SimpleService.Start();
      SimpleService.Pause();
      SimpleService.Continue();
      SimpleService.Stop();
      SimpleService.Uninstall();
    }
    ```

  - Service Process

    ```C++
    #include <Win32Ex/System/Service.hpp>

    using namespace Win32Ex::System;

    Service SimpleService("SimpleSvc");

    int main()
    {
        Service::Instance<SimpleService>::Get()
            .OnStart([]() {
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
            .Run();
    }
    ```

- Not stoppable service
  - Control Process

    ```C++
    Win32Ex::System::Service TestService("TestSvc");

    #define TEST_SVC_USER_CONTROL_ACCEPT_STOP 130

    int main(int argc, char* argv[])
    {
      TestService.SetAcceptStop([]() -> bool { return Test2Service.Control(TEST_SVC_USER_CONTROL_ACCEPT_STOP); });
      TestService.Stop();
      TestService.Uninstall();
    }
    ```

  - Service Process

    ```C++
    Win32Ex::System::Service TestService("TestSvc");
    typedef Win32Ex::System::Service::Instance<TestService> TestServiceInstance;

    #define TEST_SVC_USER_CONTROL_ACCEPT_STOP 130

    int main(int argc, char *argv[])
    {
        TestServiceInstance &svc = TestServiceInstance::Get();
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
            .On(TEST_SVC_USER_CONTROL_ACCEPT_STOP,
                [&svc]() {
                    if (!svc.SetControlsAccepted(SERVICE_ACCEPT_STOP))
                    {
                        // TODO
                    }
                    // TODO
                })
            .Run();
    }
    ```

- Share process service
  - Control Process

    ```C++
    Win32Ex::System::Service TestService("TestSvc");
    Win32Ex::System::Service Test2Servic("Test2Svc");

    int main()
    {
      // ...
      TestService.Install(SERVICE_WIN32_SHARE_PROCESS, // ... );
      TestService.Start();

      Test2Service.Install(SERVICE_WIN32_SHARE_PROCESS, // ... );
      TestService.Start();
      //...
    }
    ```

  - Service Process

    ```C++
    using namespace Win32Ex::System;

    Service TestService("TestSvc");
    typedef Service::Instance<TestService> TestServiceInstance;

    Service Test2Service("Test2Svc");
    typedef Service::Instance<Test2Service> Test2ServiceInstance;

    int main()
    {
      TestServiceInstance &svc = TestService::Get();
      Test2ServiceInstance &svc2 = Test2Service::Get();

      // ...

      return Service::Run(svc, svc2) ? EXIT_SUCCESS : EXIT_FAILURE;
      // You may use 'Service::Run<TestServiceInstance, Test2ServiceInstance>()' instead.
    }
    ```
