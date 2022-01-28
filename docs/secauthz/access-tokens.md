# Access Tokens

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/secauthz/access-tokens>
- **Headers :** Win32Ex/Security/Token.h, Win32Ex/Security/Token.hpp

## Contents

- [Access Tokens](#access-tokens)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Functions](#functions)
      - [GetTokenInfo](#gettokeninfo)
      - [FreeTokenInfo](#freetokeninfo)
      - [GetTokenUser](#gettokenuser)
      - [FreeTokenUser](#freetokenuser)
      - [GetTokenGroups](#gettokengroups)
      - [FreeTokenGroups](#freetokengroups)
      - [GetProcessTokenUserSid](#getprocesstokenusersid)
      - [DuplicateTokenUserSid](#duplicatetokenusersid)
      - [FreeTokenUserSid](#freetokenusersid)
      - [IsUserAdmin](#isuseradmin)
      - [EqualTokenUserSid](#equaltokenusersid)
      - [IsNetworkServiceToken](#isnetworkservicetoken)
      - [IsLocalServiceToken](#islocalservicetoken)
      - [IsLocalSystemToken](#islocalsystemtoken)
      - [LookupToken](#lookuptoken)
      - [LookupToken2](#lookuptoken2)
      - [LookupTokenEx](#lookuptokenex)
      - [LookupTokenEx2](#lookuptokenex2)
      - [GetLocalSystemToken](#getlocalsystemtoken)
    - [Classes](#classes)
      - [Token](#token)

## Reference

### Functions

#### GetTokenInfo

#### FreeTokenInfo

#### GetTokenUser

#### FreeTokenUser

#### GetTokenGroups

#### FreeTokenGroups

#### GetProcessTokenUserSid

#### DuplicateTokenUserSid

#### FreeTokenUserSid

#### IsUserAdmin

- [Reference](<https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership>)

#### EqualTokenUserSid

#### IsNetworkServiceToken

#### IsLocalServiceToken

#### IsLocalSystemToken

#### LookupToken

- Example
  - Find the token containing the group logon id.

    ```C
    #include <Win32Ex/System/Token.h>

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

#### LookupToken2

- Similar to LookupToken, but includes the process id in the condition function.

#### LookupTokenEx

- Similar to LookupToken, but includes the context in the condition function.

#### LookupTokenEx2

- Similar to LookupToken, but includes the process id and context in the condition function.

#### GetLocalSystemToken

- Example
  - Get a local system token.

    ```C
    #include <Win32Ex/System/Token.h>

    HANDLE token = GetLocalSystemToken(TOKEN_QUERY | TOKEN_READ | TOKEN_IMPERSONATE | TOKEN_QUERY_SOURCE |
                                        TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_EXECUTE);

    // TODO
    
    CloseHandle(token);
    ```

### Classes

#### Token

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
    #include <Win32Ex/System/Token.hpp>
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
    #include <Win32Ex/System/Token.hpp>
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
