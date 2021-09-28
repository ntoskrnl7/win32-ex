# win32-ex

[![CMake](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml) [![MSYS2](https://github.com/ntoskrnl7/win32-ex/actions/workflows/msys2.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/msys2.yml)

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
      - [Windows System Information [Handles and Objects]](#windows-system-information-handles-and-objects)
        - [Reference](#reference-1)
          - [Functions](#functions-1)
          - [Example](#example-1)
    - [Security and Identity](#security-and-identity)
      - [Authorization [Privileges]](#authorization-privileges)
        - [Reference](#reference-2)
          - [Functions](#functions-2)
          - [Macros](#macros)
          - [Example](#example-2)
      - [Authorization [Access Tokens]](#authorization-access-tokens)
        - [Reference](#reference-3)
          - [Functions](#functions-3)
          - [Example](#example-3)
      - [Authorization [Security Descriptors]](#authorization-security-descriptors)
        - [Reference](#reference-4)
          - [Functions](#functions-4)
      - [Authorization [Security Identifiers]](#authorization-security-identifiers)
        - [Reference](#reference-5)
          - [Functions](#functions-5)
          - [Variables](#variables)
  - [Test](#test)
  - [Usage](#usage)
    - [CMakeLists.txt](#cmakeliststxt)

## Features

### System Services

#### Processes and Threads

- **Link :** https://docs.microsoft.com/en-us/windows/win32/procthread/processes-and-threads
- **Headers :** System\Process.h, System\Process.hpp

##### Reference

###### Functions

- CreateSystemAccountProcess
- CreateUserAccountProcess

###### Classes

- SystemAccountProcess
- UserAccountProcess

###### Example

C/C++

```C
#include <System\Process.h>

CreateUserAccountProcess(WTSGetActiveConsoleSessionId(), NULL, "CMD /C QUERY SESSION", /* ... */);

CreateSystemAccountProcess(WTSGetActiveConsoleSessionId(), NULL,"CMD /C QUERY SESSION", /* ... */);
```

C++

```CPP
#include <System\Process.hpp>

Win32Ex::System::UserAccountProcess process(WTSGetActiveConsoleSessionId(), "CMD /C QUERY SESSION");
process.Run();

Win32Ex::System::SystemAccountProcess process(WTSGetActiveConsoleSessionId(), "CMD /C QUERY SESSION");
process.Run();
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
#include <System\Object.h>

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
#include <System\Privilege.h>
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
#include <System\Privilege.hpp>
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
#include <System\Token.hpp>
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
CPMAddPackage("gh:ntoskrnl7/win32-ex@0.1.0")

# link dependencies
target_link_libraries(tests win32ex)
```
