# win32-ex

[![CMake](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml)

Win32 API Experimental(or Extension) features

## Contents
- [win32-ex](#win32-ex)
  - [Contents](#contents)
  - [Features](#features)
    - [System Services](#system-services)
      - [Windows System Information [Handles and Objects]](#windows-system-information-handles-and-objects)
        - [Reference](#reference)
          - [Functions](#functions)
          - [Example](#example)
    - [Security and Identity](#security-and-identity)
      - [Authorization [Privileges]](#authorization-privileges)
        - [Reference](#reference-1)
          - [Functions](#functions-1)
          - [Macros](#macros)
          - [Example](#example-1)
      - [Authorization [Access Tokens]](#authorization-access-tokens)
        - [Reference](#reference-2)
          - [Functions](#functions-2)
      - [Authorization [Security Identifiers]](#authorization-security-identifiers)
        - [Reference](#reference-3)
          - [Variables](#variables)
- [Test](#test)

## Features
### System Services
####  Windows System Information [Handles and Objects]
- **Link :** https://docs.microsoft.com/en-us/windows/win32/sysinfo/handles-and-objects
- **Headers :** System\Object.h
##### Reference
###### Functions
- MakePermanentObject
- MakeTemporaryObject
- IsPermanentObject
- IsTemporaryObject

###### Example
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
- **Link :** https://docs.microsoft.com/en-us/windows/win32/secauthz/privileges
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
#include <System\Privilege.h>
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

#### Authorization [Security Identifiers]
- **Link :** https://docs.microsoft.com/en-us/windows/win32/secauthz/security-identifiers
- **Headers :** Security\Sid.h

##### Reference
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

# Test
1. cd test
2. mkdir build && cd build
3. cmake ..
4. cmake --build .
5. Debug\unittest.exe
