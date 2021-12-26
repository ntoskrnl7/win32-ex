# win32-ex

[![CMake](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml) [![MSYS2](https://github.com/ntoskrnl7/win32-ex/actions/workflows/msys2.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/msys2.yml) ![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/ntoskrnl7/win32-ex)

Win32 API Experimental(or Extension) features

## Requirements

- Windows 8 or later
- Visual Studio 2008 or later

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
          - [Macros](#macros)
          - [Example](#example-3)
      - [Authorization [Access Tokens]](#authorization-access-tokens)
        - [Reference](#reference-4)
          - [Functions](#functions-3)
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
        - [Classes](#classes-2)
      - [Win32 Api Template](#win32-api-template)
        - [ShellApi](#shellapi)
          - [Functions](#functions-6)
          - [Structures](#structures)
          - [Example](#example-5)
        - [Processes and Threads](#processes-and-threads-1)
          - [Functions](#functions-7)
          - [Structures](#structures-1)
          - [Example](#example-6)
        - [TlHelp32](#tlhelp32)
          - [Functions](#functions-8)
          - [Structures](#structures-2)
          - [Example](#example-7)
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
- CreateUserAccountProcessT\<typename CharType\> (for C++)
- CreateSystemAccountProcessT\<typename CharType\> (for C++)

###### Classes

- Process
- ProcessW
- ProcessT
- BasicProcess\<class StringType\>
- RunnableProcess
- RunnableProcessW
- RunnableProcessT
- BasicRunnableProcess\<class StringType\>
- UserAccountProcess
- UserAccountProcessW
- UserAccountProcessT
- SystemAccountProcess
- SystemAccountProcessW
- SystemAccountProcessT
- BasicRunnableSessionProcess\<class StringType, ProcessAccountType Type\>
- ElevatedProcess
- ElevatedProcessW
- ElevatedProcessT
- BasicElevatedProcess\<class StringType\>

###### Example

C/C++

```C
#include <Win32Ex\System\Process.h>

CreateUserAccountProcess(WTSGetActiveConsoleSessionId(), NULL, TEXT("CMD.exe /C QUERY SESSION"), /* ... */);

CreateSystemAccountProcess(WTSGetActiveConsoleSessionId(), NULL, TEXT("CMD.exe /C QUERY SESSION"), /* ... */);
```

C++

```C++
#include <Win32Ex\System\Process.h>

CreateUserAccountProcessT<CHAR>(WTSGetActiveConsoleSessionId(), NULL, "CMD.exe /C QUERY SESSION", /* ... */);

CreateSystemAccountProcessT<CHAR>(WTSGetActiveConsoleSessionId(), NULL, "CMD.exe /C QUERY SESSION", /* ... */);
```

```C++
#include <Win32Ex\System\Process.hpp>

Win32Ex::System::UserAccountProcess process(WTSGetActiveConsoleSessionId(), "CMD.exe /C QUERY SESSION");
process.Run();

Win32Ex::System::SystemAccountProcess process(WTSGetActiveConsoleSessionId(), "CMD.exe /C QUERY SESSION");
process.Run();

Win32Ex::System::ElevatedProcess process("notepad.exe");
process.Run();
```

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

- ServiceConfig
- ServiceConfigW
- ServiceConfigT
- BasicServiceConfig\<class StringType\>
- Service\<ServiceConfig Config\>
- ServiceW\<ServiceConfigW Config\>
- ServiceT\<ServiceConfigW Config\>
- BasicService\<class StringType, ServiceConfigW Config\>
- Services
- ServicesW
- ServicesT
- BasicServicesT\<class StringType\>

###### Example

C++

Simple service (Control Process)

```C++
#include <Win32Ex\System\Service.hpp>

Win32Ex::System::ServiceConfig SimpleServiceConfig("SimpleSvc");

int main()
{
  SimpleServiceConfig.Install();
  SimpleServiceConfig.Start();
  SimpleServiceConfig.Pause();
  SimpleServiceConfig.Continue();
  SimpleServiceConfig.Stop();
  SimpleServiceConfig.Uninstall();
}
```

Simple service (Service Process)

```C++
#include <Win32Ex\System\Service.hpp>

Win32Ex::System::ServiceConfig SimpleServiceConfig("SimpleSvc");
typedef Win32Ex::System::Service<SimpleServiceConfig> SimpleService;

int main()
{
  SimpleService &svc = SimpleService::Instance();
  svc.OnStart([]() {
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
Win32Ex::System::ServiceConfig TestServiceConfig("TestSvc");

#define TEST_SVC_USER_CONTROL_ACCEPT_STOP 130

int main(int argc, char* argv[])
{
  TestServiceConfig.SetAcceptStop([]() -> bool
  {
    SC_HANDLE serviceHandle = config.ServiceHandle(SERVICE_USER_DEFINED_CONTROL);
    if (serviceHandle == NULL)
      return false;
    SERVICE_STATUS ss = { 0 };
    BOOL result = ControlService(serviceHandle, TEST_SVC_USER_CONTROL_ACCEPT_STOP, &ss);
    CloseServiceHandle(serviceHandle);
    return (result == TRUE);
  });

  TestServiceConfig.Stop();
  TestServiceConfig.Uninstall();
}
```

Not stoppable service (Service Process)

```C++
Win32Ex::System::ServiceConfig TestServiceConfig("TestSvc");
typedef Win32Ex::System::Service<TestServiceConfig> TestService;

#define TEST_SVC_USER_CONTROL_ACCEPT_STOP 130

int main(int argc, char* argv[])
{
  TestService &svc = TestService::Instance();
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
Win32Ex::System::ServiceConfig TestServiceConfig("TestSvc");
Win32Ex::System::ServiceConfig Test2ServiceConfig("Test2Svc");

int main()
{
  // ...
  TestServiceConfig.Install(SERVICE_WIN32_SHARE_PROCESS, // ... );
  TestServiceConfig.Start();

  Test2ServiceConfig.Install(SERVICE_WIN32_SHARE_PROCESS, // ... );
  TestServiceConfig.Start();
  //...
}
```

Share process service (Service Process)

```C++
Win32Ex::System::ServiceConfig TestServiceConfig("TestSvc");
typedef Win32Ex::System::Service<TestServiceConfig> TestService;

Win32Ex::System::ServiceConfig Test2ServiceConfig("Test2Svc");
typedef Win32Ex::System::Service<Test2ServiceConfig> Test2Service;

int main()
{
  TestService &svc = TestService::Instance();
  TestService &svc2 = Test2Service::Instance();

  // ...

  return Win32Ex::System::Services::Run(svc, svc2) ? EXIT_SUCCESS : EXIT_FAILURE;
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

###### Macros

- SE_MIN_WELL_KNOWN_PRIVILEGE
- SE_MAX_WELL_KNOWN_PRIVILEGE

###### Example

```C++
#include <Win32Ex\System\Privilege.h>
using namespace Win32Ex;

Security::TokenPrivileges priv({Security::SeDebugPrivilege, Security::SeShutdownPrivilege});
if (priv.AcquiredPrivileges().size() == 2){
    // TODO
}

if (priv.IsAcquired()) {
    // TODO
}
```

OR

```C++
#include <Win32Ex\System\Privilege.hpp>
using namespace Win32Ex;

{
  Security::TokenPrivileges priv(Security::SeDebugPrivilege);
  if (priv.IsAcquired()) {
      // TODO
  }
}

// Debug privilege released.

{
  Security::TokenPrivileges priv(Security::SeDebugPrivilege);
  if (priv.IsAcquired()) {
      // TODO
      priv.SetPermanent(TRUE);
  }
}

// Debug privilege still acquired.

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

###### Example

C++

```C++
#include <Win32Ex\System\Token.hpp>
using namespace Win32Ex;

Security::Token token = Security::Token::Current();
if (token.IsValid()) {
  Security::TokenPrivileges privs = token.AdjustPrivileges({Security::SeShutdownPrivilege});
  // TODO
  if (token.IsAcquired(Security::SeShutdownPrivilege)) {
    //...
  }
  // ...
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
CPMAddPackage("gh:ntoskrnl7/win32-ex@0.8.2")

# link dependencies
target_link_libraries(tests win32ex)
```
