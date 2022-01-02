# win32-ex

[![CMake](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml) [![MSYS2](https://github.com/ntoskrnl7/win32-ex/actions/workflows/msys2.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/msys2.yml) ![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/ntoskrnl7/win32-ex)

Win32 API Experimental(or Extension) features

## Requirements

- Windows 8 or later
- Visual Studio 2008 SP1 or later
- MSYS2
  - MSYS
  - MinGW 32-bit
  - MinGW 64-bit
  - MinGW Clang 32-bit
  - MinGW Clang 64-bit
  - MinGW UCRT 64-bit

## Contents

- [win32-ex](#win32-ex)
  - [Requirements](#requirements)
  - [Contents](#contents)
  - [Features](#features)
    - [System Services](#system-services)
      - [Processes and Threads](#processes-and-threads)
        - [Reference](#reference)
          - [Functions](#functions)
          - [Classes](#classes)
          - [Namespaces](#namespaces)
      - [Remote Desktop Services](#remote-desktop-services)
        - [Reference](#reference-1)
          - [Classes](#classes-1)
          - [Namespaces](#namespaces-1)
      - [Services](#services)
        - [Reference](#reference-2)
          - [Classes](#classes-2)
          - [Example](#example)
      - [Windows System Information [Handles and Objects]](#windows-system-information-handles-and-objects)
        - [Reference](#reference-3)
          - [Functions](#functions-1)
          - [Example](#example-1)
    - [Security and Identity](#security-and-identity)
      - [Authorization [Privileges]](#authorization-privileges)
        - [Reference](#reference-4)
          - [Functions](#functions-2)
          - [Classes](#classes-3)
          - [Macros](#macros)
      - [Authorization [Access Tokens]](#authorization-access-tokens)
        - [Reference](#reference-5)
          - [Functions](#functions-3)
          - [Classes](#classes-4)
      - [Authorization [Security Descriptors]](#authorization-security-descriptors)
        - [Reference](#reference-6)
          - [Functions](#functions-4)
      - [Authorization [Security Identifiers]](#authorization-security-identifiers)
        - [Reference](#reference-7)
          - [Functions](#functions-5)
          - [Variables](#variables)
    - [Etc](#etc)
      - [Optional](#optional)
        - [Classes](#classes-5)
        - [Example](#example-2)
      - [Result](#result)
        - [Classes](#classes-6)
        - [Example](#example-3)
      - [Win32 Api Template](#win32-api-template)
        - [ShellApi](#shellapi)
          - [Functions](#functions-6)
          - [Structures](#structures)
          - [Example](#example-4)
        - [Processes and Threads](#processes-and-threads-1)
          - [Functions](#functions-7)
          - [Structures](#structures-1)
          - [Example](#example-5)
        - [TlHelp32](#tlhelp32)
          - [Functions](#functions-8)
          - [Structures](#structures-2)
          - [Example](#example-6)
  - [Test](#test)
  - [Usage](#usage)
    - [CMakeLists.txt](#cmakeliststxt)

## Features

### System Services

#### Processes and Threads

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/procthread/processes-and-threads>
- **Headers :** Win32Ex\System\Process.h, Win32Ex\System\Process.hpp

##### Reference

###### Functions

- CreateUserAccountProcess
  - Example
    - Runs user account process at the current session.

      ```C
      #include <Win32Ex\System\Process.h>

      CreateUserAccountProcess(WTSGetActiveConsoleSessionId(), NULL, TEXT("CMD.exe /C QUERY SESSION"), /* ... */);    
      ```

- CreateSystemAccountProcess
  - Example
    - Runs system account process at the current session.

      ```C
      #include <Win32Ex\System\Process.h>

      CreateSystemAccountProcess(WTSGetActiveConsoleSessionId(), NULL, TEXT("CMD.exe /C QUERY SESSION"), /* ... */);    
      ```

- CreateUserAccountProcessT\<typename CharType\>
  - **C++ only**
  - Example
    - Runs user account process at the current session.

      ```C++
      #include <Win32Ex\System\Process.h>

      CreateUserAccountProcessT<CHAR>(WTSGetActiveConsoleSessionId(), NULL, "CMD.exe /C QUERY SESSION", /* ... */);
      ```

- CreateSystemAccountProcessT\<typename CharType\>
  - **C++ only**
  - Example
    - Runs system account process at the current session.

      ```C++
      #include <Win32Ex\System\Process.h>

      CreateSystemAccountProcessT<CHAR>(WTSGetActiveConsoleSessionId(), NULL, "CMD.exe /C QUERY SESSION", /* ... */);
      ```

###### Classes

- Process
  - Example
    - Enumerate all processes.

      ```C++
      #include <Win32Ex\System\Process.hpp>

      for (auto process : Win32Ex::System::Process::All())
      {
          if (process)
          {
              if (process->IsAttached())
                  std::cout << "[Attached] PID :" << process->Id() << "\t\tPATH : " << process->ExecutablePath() << '\n';
              else
                  std::cout << "PID :" << process->Id() << "\t\tPATH : " << process->ExecutablePath() << '\n';
          }
      }
      ```

- ProcessW
- ProcessT\<class StringType = StringT\>
- RunnableProcess
  - **Abstract**
  - Example
    - Use the RunnableProcess class to run 'user account process'.

      ```C++
      #include <Win32Ex\System\Process.hpp>

      Win32Ex::System::UserAccountProcess process("CMD", "/C WHOAMI");
      Win32Ex::System::RunnableProcess &runnable = process;
      runnable.Run();
      ```

- RunnableProcessW
  - **Abstract**
- RunnableProcessT\<class StringType = StringT\>
  - **Abstract**
- UserAccountProcess
  - Example
    - Runs user account process at the current session.

      ```C++
      #include <Win32Ex\System\Process.hpp>

      Win32Ex::System::UserAccountProcess process("CMD.exe /C QUERY SESSION");
      process.Run();
      ```

    - Runs user account process at each sessions.

      ```C++
      #include <Win32Ex\System\Process.hpp>

      PWTS_SESSION_INFO sessionInfo = NULL;
      DWORD count = 0;
      if (WTSEnumerateSessions(WTS_CURRENT_SERVER, 0, 1, &sessionInfo, &count))
      {
        for (DWORD i = 0; i < count; ++i)
        {
          if (sessionInfo[i].State == WTSListen)
            continue;

          Win32Ex::System::UserAccountProcess userProcess(sessionInfo[i].SessionId, "CMD", "/C QUERY SESSION");
          userProcess.Run();
        }
      }
      ```

- UserAccountProcessW
- UserAccountProcessT
- SystemAccountProcess
- SystemAccountProcessW
- SystemAccountProcessT
- RunnableSessionProcessT\<ProcessAccountType Type, class StringType = StringT\>
- ElevatedProcess
  - Example
    - Run with elevated permissions UAC.

      ```C++
      #include <Win32Ex\System\Process.hpp>

      Win32Ex::System::ElevatedProcess process("notepad.exe");
      process.Run();
      ```

- ElevatedProcessW
- ElevatedProcessT\<class StringType = StringT\>

###### Namespaces

- Win32Ex::ThisProcess
  - Example
    - Enumerate parent processes.

      ```C++
      Win32Ex::System::Process parent = Win32Ex::ThisProcess::Parent();
      while (parent.IsValid())
      {
          std::cout << parent.ExecutablePath() << '\n';
          parent = parent.Parent();
      }
      ```

#### Remote Desktop Services

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/termserv/terminal-services-portal>
- **Headers :** System\Session.hpp

##### Reference

###### Classes

- Session
  - Example
    - Runs system account process and user account process at each sessions.

      ```C++
      for (auto session : Win32Ex::System::Session::All())
      {
          auto process = session.NewProcess(Win32Ex::System::UserAccount, "notepad");
          if (process.IsOk())
          {
              process->RunAsync().Wait(500);
              process->Exit();
          }
          process = session.NewProcess(Win32Ex::System::SystemAccount, "notepad");
          if (process.IsOk())
          {
              process->RunAsync().Wait(500);
              process->Exit();
          }
      }
      ```

- SessionW
- SessionT\<class StringType = StringT\>

###### Namespaces

- Win32Ex::ThisSession
  - Example
    - Runs system account process and user account process at this session.

      ```C++
      auto process = Win32Ex::ThisSession::NewProcess(Win32Ex::System::UserAccount, "notepad");
      if (process.IsOk())
      {
          process->RunAsync().Wait(500);
          process->Exit();
      }
      process = Win32Ex::ThisSession::NewProcess(Win32Ex::System::SystemAccount, "notepad");
      if (process.IsOk())
      {
          process->RunAsync().Wait(500);
          process->Exit();
      }
      ```

#### Services

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/services/services>
- **Headers :** Win32Ex\System\Service.hpp

##### Reference

###### Classes

- Service
- ServiceW
- ServiceT\<class StringType = Win32Ex::StringT\>
- Service::Instance\<Service\>
  - **Singleton**
- ServiceW::Instance\<ServiceW\>
  - **Singleton**
- ServiceT::Instance\<ServiceT\>
  - **Singleton**

###### Example

C++

```C++
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

Simple service (Control Process)

```C++
#include <Win32Ex\System\Service.hpp>

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

Simple service (Service Process)

```C++
#include <Win32Ex\System\Service.hpp>

using namespace Win32Ex::System;

Service SimpleService("SimpleSvc");

int main()
{
    Service::Instance<SimpleService>::GetInstance()
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

Not stoppable service (Control Process)

```C++
Win32Ex::System::Service TestService("TestSvc");

#define TEST_SVC_USER_CONTROL_ACCEPT_STOP 130

int main(int argc, char* argv[])
{
  TestService.SetAcceptStop([]() -> bool
  {
    SC_HANDLE serviceHandle = TestService.ServiceHandle(SERVICE_USER_DEFINED_CONTROL);
    if (serviceHandle == NULL)
      return false;
    SERVICE_STATUS ss = { 0 };
    BOOL result = ControlService(serviceHandle, TEST_SVC_USER_CONTROL_ACCEPT_STOP, &ss);
    CloseServiceHandle(serviceHandle);
    return (result == TRUE);
  });

  TestService.Stop();
  TestService.Uninstall();
}
```

Not stoppable service (Service Process)

```C++
Win32Ex::System::Service TestService("TestSvc");
typedef Win32Ex::System::Service::Instance<TestService> TestServiceInstance;

#define TEST_SVC_USER_CONTROL_ACCEPT_STOP 130

int main(int argc, char *argv[])
{
    TestServiceInstance &svc = Win32Ex::System::Service::Instance<TestService>::GetInstance();
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

Share process service (Control Process)

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

Share process service (Service Process)

```C++
using namespace Win32Ex::System;

Service TestService("TestSvc");
typedef Service::Instance<TestService> TestServiceInstance;

Service Test2Service("Test2Svc");
typedef Service::Instance<Test2Service> Test2ServiceInstance;

int main()
{
  TestServiceInstance &svc = TestService::GetInstance();
  Test2ServiceInstance &svc2 = Test2Service::GetInstance();

  // ...

  return Service::Run(svc, svc2) ? EXIT_SUCCESS : EXIT_FAILURE;
  // You may use 'Service::Run<TestServiceInstance, Test2ServiceInstance>()' instead.
}
```

#### Windows System Information [Handles and Objects]

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/sysinfo/handles-and-objects>
- **Headers :** Win32Ex\System\Object.h

##### Reference

###### Functions

- MakePermanentObject
- MakeTemporaryObject
- IsPermanentObject
- IsTemporaryObject

###### Example

C/C++

```C
#include <Win32Ex\System\Object.h>

HANDLE handle = ....
MakePermanentObject(handle); // Administrator privilege required
IsPermanentObject(handle); // == TRUE
MakeTemporaryObject(handle);
IsTemporaryObject(handle); // == TRUE
```

---

### Security and Identity

#### Authorization [Privileges]

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/secauthz/privileges>
- **Headers :** Win32Ex\Security\Privilege.h, Win32Ex\Security\Privilege.hpp

##### Reference

###### Functions

- GetTokenPrivileges
- FreeTokenPrivileges
- LookupPrivilegesValue
- EnableAvailablePrivileges
  - Example
    - Enable the avaliable privileges.

      ```C
      #include <Win32Ex/Security/Privilege.h>

      PREVIOUS_TOKEN_PRIVILEGES prev;
      if (EnableAvailablePrivileges(TRUE, &prev, NULL))
      {
        // TODO
        RevertPrivileges(&prev)
      }
      ```

- EnablePrivilege
  - Example
    - Enable the shutdown privilege.

      ```C
      #include <Win32Ex/Security/Privilege.h>

      PREVIOUS_TOKEN_PRIVILEGES prev;
      if (EnablePrivilege(TRUE, SE_SHUTDOWN_NAME, &prev, NULL))
      {
        // TODO
        RevertPrivileges(&prev)
      }
      ```

- EnablePrivilegeEx
  - Similar to EnablePrivilege, but uses LUID instead of privilege name.
- EnablePrivileges
  - Example
    - Enable change notify and shutdown privileges.

      ```C
      #include <Win32Ex/Security/Privilege.h>

      PREVIOUS_TOKEN_PRIVILEGES prev;
      PCTSTR privileges = {
        SE_CHANGE_NOTIFY_NAME,
        SE_SHUTDOWN_NAME
      };
      if (EnablePrivileges(TRUE, 2, privileges, &prev, NULL))
      {
        // TODO
        RevertPrivileges(&prev)
      }
      ```

- EnablePrivilegesV
  - Example
    - Enable change notify and shutdown privileges.

      ```C
      #include <Win32Ex/Security/Privilege.h>

      PREVIOUS_TOKEN_PRIVILEGES prev;
      if (!EnablePrivilegesV(TRUE, &prev, NULL, 2, SE_CHANGE_NOTIFY_NAME, SE_SHUTDOWN_NAME))
      {
        // TODO
        RevertPrivileges(&prev)
      }
      ```

- EnablePrivilegesEx
  - Similar to EnablePrivileges, but uses LUIDs instead of privilege names.
- EnablePrivilegesExV
  - Similar to EnablePrivilegeV, but uses LUIDs instead of privilege names.
- IsPrivilegeEnabled
  - Example
    - Determines whether the specified privilege is enabled.

      ```C
      #include <Win32Ex/Security/Privilege.h>

      if (IsPrivilegeEnabled(SE_CHANGE_NOTIFY_NAME, NULL))
      {
      }
      ```

- IsPrivilegeEnabledEx
  - Similar to IsPrivilegeEnabled, but uses LUID instead of privilege name.
  - Example
    - Determines whether the specified privilege is enabled.

      ```C
      #include <Win32Ex/Security/Privilege.h>

      if (IsPrivilegeEnabledEx(Win32Ex::Security::SeChangeNotifyPrivilege, NULL))
      {
      }
      ```

- IsPrivilegesEnabled
  - Example
    - Determines whether the change notify and shutdown privileges is enabled.

      ```C
      #include <Win32Ex/Security/Privilege.h>

      PCTSTR privileges = {
        SE_CHANGE_NOTIFY_NAME,
        SE_SHUTDOWN_NAME
      };

      if (IsPrivilegesEnabled(2, privileges, NULL))
      {
      }
      ```

- IsPrivilegesEnabledV
  - Example
    - Determines whether the change notify and shutdown privileges is enabled.

      ```C
      #include <Win32Ex/Security/Privilege.h>

      if (IsPrivilegesEnabledV(NULL, 2, SE_CHANGE_NOTIFY_NAME, SE_SHUTDOWN_NAME))
      {
      }
      ```

- IsPrivilegesEnabledEx
  - Similar to IsPrivilegesEnabled, but uses LUIDs instead of privilege names.
- IsPrivilegesEnabledExV
  - Similar to IsPrivilegesEnabledV, but uses LUIDs instead of privilege names.

###### Classes

- TokenPrivileges
  - Example
    - Adjust a debug and shutdown privileges.

      ```C++
      #include <Win32Ex\System\Privilege.h>
      using namespace Win32Ex;

      Security::TokenPrivileges priv({Security::SeDebugPrivilege, Security::SeShutdownPrivilege});
      if (priv.AcquiredPrivileges().size() == 2)
      {
          // TODO
      }

      if (priv.IsAcquired())
      {
          // TODO
      }
      ```

    - Adjust debug privilege.

      ```C++
      #include <Win32Ex\System\Privilege.hpp>
      using namespace Win32Ex;

      {
        Security::TokenPrivileges priv(Security::SeDebugPrivilege);
        if (priv.IsAcquired())
        {
          // TODO
        }
      }

      // Debug privilege released.
      ```

      ```C++
      #include <Win32Ex\System\Privilege.hpp>
      using namespace Win32Ex;

      {
        Security::TokenPrivileges priv(Security::SeDebugPrivilege);
        if (priv.IsAcquired())
        {
          priv.SetPermanent(TRUE);
          // TODO
        }

        priv.Release();

        // Debug privilege released.
      }
      ```

###### Macros

- SE_MIN_WELL_KNOWN_PRIVILEGE
- SE_MAX_WELL_KNOWN_PRIVILEGE

#### Authorization [Access Tokens]

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/secauthz/access-tokens>
- **Headers :** Win32Ex\Security\Token.h, Win32Ex\Security\Token.hpp

##### Reference

###### Functions

- GetTokenInfo
- FreeTokenInfo
- GetTokenUser
- FreeTokenUser
- GetTokenGroups
- FreeTokenGroups
- GetProcessTokenUserSid
- DuplicateTokenUserSid
- FreeTokenUserSid
- IsUserAdmin (<https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership>)
- EqualTokenUserSid
- IsNetworkServiceToken
- IsLocalServiceToken
- IsLocalSystemToken
- LookupToken
  - Example
    - Find the token containing the group logon id.

      ```C
      #include <Win32Ex\System\Token.h>

      HANDLE token = LookupToken(TOKEN_QUERY, [](HANDLE TokenHandle) -> BOOL {
          Security::Token token(TokenHandle, false);
          for (auto &group : token.Groups())
          {
              if ((group.Attributes & SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID)
                  return TRUE;
          }
          return FALSE;
      });

      // TODO
      
      CloseHandle(token);
      ```

- LookupToken2
  - Similar to LookupToken, but includes the process id in the condition function.
- LookupTokenEx
  - Similar to LookupToken, but includes the context in the condition function.
- LookupTokenEx2
  - Similar to LookupToken, but includes the process id and context in the condition function.
- GetLocalSystemToken
  - Example
    - Get a local system token.

      ```C
      #include <Win32Ex\System\Token.h>

      HANDLE token = GetLocalSystemToken(TOKEN_QUERY | TOKEN_READ | TOKEN_IMPERSONATE | TOKEN_QUERY_SOURCE |
                                          TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_EXECUTE);

      // TODO
      
      CloseHandle(token);
      ```

###### Classes

- Token
  - Example
    - New token object by token handle.

      ```C++
      HANDLE tokenHandle;
      if (!OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &tokenHandle))
        return;

      Security::Token token(tokenHandle);

      // TODO
      ```

      ```C++
      HANDLE tokenHandle;
      if (!OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &tokenHandle))
        return;

      Security::Token token(tokenHandle, false);

      // TODO

      CloseHandle(tokenHandle);
      ```

    - New token object (Lookup a token with create permanent privilege).

      ```C++
      Security::Token token([&privilegeSet](DWORD /*ProcessId*/, HANDLE TokenHandle) -> BOOL {
          PRIVILEGE_SET privilegeSet;
          privilegeSet.PrivilegeCount = 1;
          privilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
          privilegeSet.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
          privilegeSet.Privilege[0].Luid = Security::SeCreatePermanentPrivilege;
          BOOL result = FALSE;
          return PrivilegeCheck(TokenHandle, &privilegeSet, &result) && result;
      });

      // TODO
      ```

    - Adjust shutdown privilege.

      ```C++
      #include <Win32Ex\System\Token.hpp>
      using namespace Win32Ex;

      Security::Token token = Security::Token::Current();
      if (token.IsValid())
      {
        Security::TokenPrivileges privs = token.AdjustPrivilege(Security::SeShutdownPrivilege);
        if (token.IsAcquired(Security::SeShutdownPrivilege))
        {
          // TODO
        }
        // TODO
      }
      ```

    - Adjust a shutdown and time zone privileges.

      ```C++
      #include <Win32Ex\System\Token.hpp>
      using namespace Win32Ex;

      Security::Token token = Security::Token::Current();
      if (token.IsValid())
      {
        Security::TokenPrivileges privs = token.AdjustPrivileges({Security::SeShutdownPrivilege, Security::SeTimeZonePrivilege});
        if (token.IsAcquired({Security::SeShutdownPrivilege, Security::SeTimeZonePrivilege}))
        {
          // TODO
        }
        // TODO
      }
      ```

#### Authorization [Security Descriptors]

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/secauthz/security-descriptors>
- **Headers :** Win32Ex\Security\Descriptor.h

##### Reference

###### Functions

- AllocateAndInitializeSecurityDescriptor
- FreeSecurityDescriptor

#### Authorization [Security Identifiers]

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/secauthz/security-identifiers>
- **Headers :** Security\Sid.h

##### Reference

###### Functions

- GetAdministratorsSid
- GetLogonSid (<https://docs.microsoft.com/en-us/windows/win32/api/securityappcontainer/nf-securityappcontainer-getappcontainernamedobjectpath>)
- FreeLogonSid

###### Variables

- EveryoneSid
- ConsoleLogonSid
- LocalSid
- CreatorOwnerSid
- CreatorGroupSid
- ServiceSid
- LocalSystemSid
- LocalServiceSid
- NetworkServiceSid

### Etc

#### Optional

- **Headers :** Win32Ex/Optional.hpp

##### Classes

- Optional\<T\>
- Optional\<class StringType\>
- Optional\<class StringTypeW\>
- Optional\<class StringTypeT\>
- Optional\<class StringType &\>
- Optional\<class StringTypeW &\>
- Optional\<class StringTypeT &\>
- Optional\<const String &\>
- Optional\<const StringW &\>
- Optional\<const StringT &\>

##### Example

```C++

#include <Win32Ex/Optional.hpp>

using namespace Win32Ex;

void TestFn(Optional<int> arg0, Optional<double> arg1 = None(), Optional<String> arg2 = None())
{
  if (arg0.IsSome())
  {
    int val = arg0;
  }

  if (arg1.IsSome())
  {
    double val = arg1;
  }

  if (arg2.IsSome())
  {
    String val = arg2;
  }

  if (IsAll(arg0, arg1, arg2))
  {
    int val0 = arg0;
    double val1 = arg1;
    String val2 = arg2;
    ...
  }

  if (IsNone(arg0, arg1, arg2))
  {
    ...
  }
}

TestFn();
TestFn(1);
TestFn(None(), 2, None());
TestFn(None(), None(), "3");
TestFn(None(), 2, "3");
TestFn(1, None(), "3");
TestFn(1, 2);
TestFn(1, 2, "3");
```

#### Result

- **Headers :** Win32Ex/Result.hpp

##### Classes

- Result\<T\>
- Result\<T &\>

##### Example

```C++

#include <Win32Ex/Result.hpp>

using namespace Win32Ex;

Result<int> GetValue(bool error)
{
    if (error)
        return Error(ERROR_INVALID_PARAMETER, "Invalid parameter");
    return 1;
}

...

int value = 0;
try
{
    value = GetValue(true).Get();
}
catch (const Error &e)
{
    e.ErrorCode; // == ERROR_INVALID_PARAMETER
    e.what();      // == "Invalid parameter"
}
catch (const std::exception &e)
{
    e.what(); // == "Invalid parameter"
}
value = GetValue(true).Get(-1); // == -1
value = GetValue(false).Get(); // == 1

```

#### Win32 Api Template

##### ShellApi

- **Headers :** Win32Ex/T/shellapi.hpp

###### Functions

- ShellExecuteExT<_CharType>

###### Structures

- SHELLEXECUTEINFOT<_CharType>

###### Example

```C++
#include <Win32Ex/T/shellapi.hpp>

SHELLEXECUTEINFOT<CHAR> sei;
ZeroMemory(&sei, sizeof(sei));
// or
// typename SHELLEXECUTEINFOT<CHAR>::Type sei = { 0, };

...

Win32Ex::ShellExecuteExT<CHAR>(&sei);
```

##### Processes and Threads

- **Headers :** Win32Ex/T/processthreadsapi.hpp

###### Functions

- CreateProcessT<_CharType>
- CreateProcessAsUserT<_CharType>
- QueryFullProcessImageNameT<_CharType>

###### Structures

- STARTUPINFOT<_CharType>
- STARTUPINFOEXT<_CharType>

###### Example

```C++
#include <Win32Ex/T/processthreadsapi.hpp>

STARTUPINFOT<CHAR> si;

...

Win32Ex::CreateProcessAsUserT<CHAR>(..., &si, ..);
```

##### TlHelp32

- **Headers :** Win32Ex/T/tlhelp32.hpp

###### Functions

- Process32FirstT<_CharType>
- Process32NextT<_CharType>

###### Structures

- PROCESSENTRY32T<_CharType>

###### Example

```C++
#include <Win32Ex/T/tlhelp32.hpp>

typename PROCESSENTRY32T<CHAR>::Type pe32 = {
    0,
};
pe32.dwSize = sizeof(pe32);

hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

if (!Process32FirstT<CHAR>(hSnapshot, &pe32))
    return;
do
{
  ...
} while (Process32NextT<CHAR>(hSnapshot, &pe32));
...

CloseHandle(hSnapshot);

```

## Test

```batch
CD test
MKDIR build && CD build
cmake ..
cmake --build .
ctest . --verbose
```

## Usage

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# create project
project(MyProject)

# add executable
add_executable(tests tests.cpp)

# add dependencies
include(cmake/CPM.cmake)
CPMAddPackage("gh:ntoskrnl7/win32-ex@0.8.10")

# link dependencies
target_link_libraries(tests win32ex)
```
