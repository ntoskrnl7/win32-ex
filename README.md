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
          - [Example](#example)
      - [Services](#services)
        - [Reference](#reference-1)
          - [Classes](#classes-1)
          - [Example](#example-1)
      - [Windows System Information [Handles and Objects]](#windows-system-information-handles-and-objects)
        - [Reference](#reference-2)
          - [Functions](#functions-1)
          - [Example](#example-2)
    - [Security and Identity](#security-and-identity)
      - [Authorization [Privileges]](#authorization-privileges)
        - [Reference](#reference-3)
          - [Functions](#functions-2)
          - [Classes](#classes-2)
          - [Macros](#macros)
          - [Example](#example-3)
      - [Authorization [Access Tokens]](#authorization-access-tokens)
        - [Reference](#reference-4)
          - [Functions](#functions-3)
          - [Classes](#classes-3)
          - [Example](#example-4)
      - [Authorization [Security Descriptors]](#authorization-security-descriptors)
        - [Reference](#reference-5)
          - [Functions](#functions-4)
      - [Authorization [Security Identifiers]](#authorization-security-identifiers)
        - [Reference](#reference-6)
          - [Functions](#functions-5)
          - [Variables](#variables)
    - [Etc](#etc)
      - [Optional](#optional)
        - [Classes](#classes-4)
        - [Example](#example-5)
      - [Win32 Api Template](#win32-api-template)
        - [ShellApi](#shellapi)
          - [Functions](#functions-6)
          - [Structures](#structures)
          - [Example](#example-6)
        - [Processes and Threads](#processes-and-threads-1)
          - [Functions](#functions-7)
          - [Structures](#structures-1)
          - [Example](#example-7)
        - [TlHelp32](#tlhelp32)
          - [Functions](#functions-8)
          - [Structures](#structures-2)
          - [Example](#example-8)
  - [Test](#test)
  - [Usage](#usage)
    - [CMakeLists.txt](#cmakeliststxt)

## Features

### System Services

#### Processes and Threads

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/procthread/processes-and-threads>
- **Headers :** System\Process.h, System\Process.hpp

##### Reference

###### Functions

- CreateUserAccountProcess
- CreateSystemAccountProcess
- CreateUserAccountProcessT\<typename CharType\>
  - C++ only
- CreateSystemAccountProcessT\<typename CharType\>
  - C++ only

###### Classes

- Process
- ProcessW
- ProcessT\<class StringType = StringT\>
- RunnableProcess
  - Abstract
- RunnableProcessW
  - Abstract
- RunnableProcessT\<class StringType = StringT\>
  - Abstract
- UserAccountProcess
- UserAccountProcessW
- UserAccountProcessT
- SystemAccountProcess
- SystemAccountProcessW
- SystemAccountProcessT
- RunnableSessionProcessT\<ProcessAccountType Type, class StringType = StringT\>
- ElevatedProcess
- ElevatedProcessW
- ElevatedProcessT\<class StringType = StringT\>

###### Example

C/C++

Runs system account process and user account process at the current session.

```C
#include <Win32Ex\System\Process.h>

CreateUserAccountProcess(WTSGetActiveConsoleSessionId(), NULL, TEXT("CMD.exe /C QUERY SESSION"), /* ... */);

CreateSystemAccountProcess(WTSGetActiveConsoleSessionId(), NULL, TEXT("CMD.exe /C QUERY SESSION"), /* ... */);
```

C++

Runs system account process and user account process at the current session.

```C++
#include <Win32Ex\System\Process.h>

CreateUserAccountProcessT<CHAR>(WTSGetActiveConsoleSessionId(), NULL, "CMD.exe /C QUERY SESSION", /* ... */);

CreateSystemAccountProcessT<CHAR>(WTSGetActiveConsoleSessionId(), NULL, "CMD.exe /C QUERY SESSION", /* ... */);
```

Runs system account process and user account process at the current session.

```C++
#include <Win32Ex\System\Process.hpp>

Win32Ex::System::UserAccountProcess process("CMD.exe /C QUERY SESSION");
process.Run();

Win32Ex::System::SystemAccountProcess process("CMD.exe /C QUERY SESSION");
process.Run();
```

Runs system account process and user account process at each sessions.

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

    Win32Ex::System::SystemAccountProcess systemProcess(sessionInfo[i].SessionId, "CMD", "/C QUERY SESSION");
    systemProcess.Run();

    Win32Ex::System::UserAccountProcess userProcess(sessionInfo[i].SessionId, "CMD", "/C QUERY SESSION");
    userProcess.Run();
  }
}
```

Run with elevated permissions UAC.

```C++
#include <Win32Ex\System\Process.hpp>

Win32Ex::System::ElevatedProcess process("notepad.exe");
process.Run();
```

Enumerate parent processes.

```C++
Win32Ex::System::Process parent = Win32Ex::ThisProcess::Parent();
while (parent.IsValid())
{
    std::cout << parent.ExecutablePath() << '\n';
    parent = parent.Parent();
}
```

#### Services

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/services/services>
- **Headers :** System\Service.hpp

##### Reference

###### Classes

- Service
- ServiceW
- ServiceT\<class StringType = Win32Ex::StringT\>
- Service::Instance\<Service\>
  - Singleton
- ServiceW::Instance\<ServiceW\>
  - Singleton
- ServiceT::Instance\<ServiceT\>
  - Singleton

###### Example

C++

```C++
Win32Ex::System::Service service("ProfSvc");

std::cout << "\n\n-----------------Dependencies-------------------\n";
for (auto &dep : service.Dependencies())
{
    std::cout << dep.Name() << "\n\t" << dep.DisplayName() << "\n\t" << dep.BinaryPathName() << '\n';
    for (auto &dep2 : dep.Dependencies())
      std::cout << "\t\t" << dep2.Name() << "\n\t\t\t" << dep2.DisplayName() << "\n\t\t\t" << dep2.BinaryPathName() << '\n';
}

std::cout << "\n\n-----------------DependentServices-------------------\n";
for (auto &dep : service.DependentServices().Get({}))
{
    std::cout << dep.Name() << "\n\t" << dep.DisplayName() << "\n\t" << dep.BinaryPathName() << '\n';
    for (auto &dep2 : dep.DependentServices().Get({}))
      std::cout << "\t\t" << dep2.Name() << "\n\t\t\t" << dep2.DisplayName() << "\n\t\t\t"
                << dep2.BinaryPathName() << '\n';
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
- **Headers :** System\Object.h

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
- **Headers :** Security\Privilege.h, Security\Privilege.hpp

##### Reference

###### Functions

- GetTokenPrivileges
- FreeTokenPrivileges
- LookupPrivilegesValue
- EnableAvailablePrivileges
- EnablePrivilege
- EnablePrivilegeEx
- EnablePrivileges
- EnablePrivilegesV
- EnablePrivilegesEx
- EnablePrivilegesExV
- IsPrivilegeEnabled
- IsPrivilegeEnabledEx
- IsPrivilegesEnabled
- IsPrivilegesEnabledV
- IsPrivilegesEnabledEx
- IsPrivilegesEnabledExV

###### Classes
- TokenPrivileges

###### Macros

- SE_MIN_WELL_KNOWN_PRIVILEGE
- SE_MAX_WELL_KNOWN_PRIVILEGE

###### Example

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

OR

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
}

// Debug privilege still acquired.

priv.Release();

// Debug privilege released.

```

#### Authorization [Access Tokens]

- **Link :** https://docs.microsoft.com/en-us/windows/win32/secauthz/access-tokens
- **Headers :** Security\Token.h, Security\Token.hpp

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
- IsUserAdmin (https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership)
- EqualTokenUserSid
- IsNetworkServiceToken
- IsLocalServiceToken
- IsLocalSystemToken
- LookupToken
- LookupToken2
- LookupTokenEx
- LookupTokenEx2
- GetLocalSystemToken

###### Classes
- Token

###### Example

C++

```C++
#include <Win32Ex\System\Token.hpp>
using namespace Win32Ex;

Security::Token token = Security::Token::Current();
if (token.IsValid())
{
  Security::TokenPrivileges privs = token.AdjustPrivileges({Security::SeShutdownPrivilege});
  if (token.IsAcquired(Security::SeShutdownPrivilege))
  {
    // TODO
  }
  // TODO
}

```

#### Authorization [Security Descriptors]

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/secauthz/security-descriptors>
- **Headers :** Security\Descriptor.h

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
- GetLogonSid
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
Debug\unittest.exe
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
CPMAddPackage("gh:ntoskrnl7/win32-ex@0.8.7")

# link dependencies
target_link_libraries(tests win32ex)
```
