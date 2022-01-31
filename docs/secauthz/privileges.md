# Privileges

- **Link :** <https://docs.microsoft.com/windows/win32/secauthz/privileges>
- **Headers :** Win32Ex/Security/Privilege.h, Win32Ex/Security/Privilege.hpp

## Contents

- [Privileges](#privileges)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Functions](#functions)
      - [GetTokenPrivileges](#gettokenprivileges)
      - [FreeTokenPrivileges](#freetokenprivileges)
      - [LookupPrivilegesValue](#lookupprivilegesvalue)
      - [EnableAvailablePrivileges](#enableavailableprivileges)
      - [EnablePrivilege](#enableprivilege)
      - [EnablePrivilegeEx](#enableprivilegeex)
      - [EnablePrivileges](#enableprivileges)
      - [EnablePrivilegesV](#enableprivilegesv)
      - [EnablePrivilegesEx](#enableprivilegesex)
      - [EnablePrivilegesExV](#enableprivilegesexv)
      - [IsPrivilegeEnabled](#isprivilegeenabled)
      - [IsPrivilegeEnabledEx](#isprivilegeenabledex)
      - [IsPrivilegesEnabled](#isprivilegesenabled)
      - [IsPrivilegesEnabledV](#isprivilegesenabledv)
      - [IsPrivilegesEnabledEx](#isprivilegesenabledex)
      - [IsPrivilegesEnabledExV](#isprivilegesenabledexv)
    - [Variables](#variables)
      - [Privilege LUID variables [*C++ Only*]](#privilege-luid-variables-c-only)
        - [SeCreateTokenPrivilege](#secreatetokenprivilege)
        - [SeAssignPrimaryTokenPrivilege](#seassignprimarytokenprivilege)
        - [SeLockMemoryPrivilege](#selockmemoryprivilege)
        - [SeIncreaseQuotaPrivilege](#seincreasequotaprivilege)
        - [SeUnsolicitedInputPrivilege](#seunsolicitedinputprivilege)
        - [SeMachineAccountPrivilege](#semachineaccountprivilege)
        - [SeTcbPrivilege](#setcbprivilege)
        - [SeSecurityPrivilege](#sesecurityprivilege)
        - [SeTakeOwnershipPrivilege](#setakeownershipprivilege)
        - [SeLoadDriverPrivilege](#seloaddriverprivilege)
        - [SeSystemProfilePrivilege](#sesystemprofileprivilege)
        - [SeSystemtimePrivilege](#sesystemtimeprivilege)
        - [SeProfileSingleProcessPrivilege](#seprofilesingleprocessprivilege)
        - [SeIncreaseBasePriorityPrivilege](#seincreasebasepriorityprivilege)
        - [SeCreatePagefilePrivilege](#secreatepagefileprivilege)
        - [SeCreatePermanentPrivilege](#secreatepermanentprivilege)
        - [SeBackupPrivilege](#sebackupprivilege)
        - [SeRestorePrivilege](#serestoreprivilege)
        - [SeShutdownPrivilege](#seshutdownprivilege)
        - [SeDebugPrivilege](#sedebugprivilege)
        - [SeAuditPrivilege](#seauditprivilege)
        - [SeSystemEnvironmentPrivilege](#sesystemenvironmentprivilege)
        - [SeChangeNotifyPrivilege](#sechangenotifyprivilege)
        - [SeRemoteShutdownPrivilege](#seremoteshutdownprivilege)
        - [SeUndockPrivilege](#seundockprivilege)
        - [SeSyncAgentPrivilege](#sesyncagentprivilege)
        - [SeEnableDelegationPrivilege](#seenabledelegationprivilege)
        - [SeManageVolumePrivilege](#semanagevolumeprivilege)
        - [SeImpersonatePrivilege](#seimpersonateprivilege)
        - [SeCreateGlobalPrivilege](#secreateglobalprivilege)
        - [SeTrustedCredManAccessPrivilege](#setrustedcredmanaccessprivilege)
        - [SeRelabelPrivilege](#serelabelprivilege)
        - [SeIncreaseWorkingSetPrivilege](#seincreaseworkingsetprivilege)
        - [SeTimeZonePrivilege](#setimezoneprivilege)
        - [SeCreateSymbolicLinkPrivilege](#secreatesymboliclinkprivilege)
        - [SeDelegateSessionUserImpersonatePrivilege](#sedelegatesessionuserimpersonateprivilege)
    - [Classes](#classes)
      - [TokenPrivileges](#tokenprivileges)
    - [Macros](#macros)
      - [Privilege macros](#privilege-macros)
        - [SE_MIN_WELL_KNOWN_PRIVILEGE](#se_min_well_known_privilege)
        - [SE_MAX_WELL_KNOWN_PRIVILEGE](#se_max_well_known_privilege)
        - [SE_PROF_SINGLE_PROCESS_NAME_W](#se_prof_single_process_name_w)
        - [SE_INC_BASE_PRIORITY_NAME_W](#se_inc_base_priority_name_w)
        - [SE_CREATE_PAGEFILE_NAME_W](#se_create_pagefile_name_w)
        - [SE_CREATE_PERMANENT_NAME_W](#se_create_permanent_name_w)
        - [SE_BACKUP_NAME_W](#se_backup_name_w)
        - [SE_RESTORE_NAME_W](#se_restore_name_w)
        - [SE_SHUTDOWN_NAME_W](#se_shutdown_name_w)
        - [SE_DEBUG_NAME_W](#se_debug_name_w)
        - [SE_AUDIT_NAME_W](#se_audit_name_w)
        - [SE_SYSTEM_ENVIRONMENT_NAME_W](#se_system_environment_name_w)
        - [SE_CHANGE_NOTIFY_NAME_W](#se_change_notify_name_w)
        - [SE_REMOTE_SHUTDOWN_NAME_W](#se_remote_shutdown_name_w)
        - [SE_UNDOCK_NAME_W](#se_undock_name_w)
        - [SE_SYNC_AGENT_NAME_W](#se_sync_agent_name_w)
        - [SE_ENABLE_DELEGATION_NAME_W](#se_enable_delegation_name_w)
        - [SE_MANAGE_VOLUME_NAME_W](#se_manage_volume_name_w)
        - [SE_IMPERSONATE_NAME_W](#se_impersonate_name_w)
        - [SE_CREATE_GLOBAL_NAME_W](#se_create_global_name_w)
        - [SE_TRUSTED_CREDMAN_ACCESS_NAME_W](#se_trusted_credman_access_name_w)
        - [SE_RELABEL_NAME_W](#se_relabel_name_w)
        - [SE_INCORKING_SET_NAME_W](#se_incorking_set_name_w)
        - [SE_TIME_ZONE_NAME_W](#se_time_zone_name_w)
        - [SE_CREATE_SYMBOLIC_LINK_NAME_W](#se_create_symbolic_link_name_w)
        - [SE_DELEGATE_SESSION_USER_IMPERSONATE_NAME_W](#se_delegate_session_user_impersonate_name_w)

## Reference

### Functions

#### GetTokenPrivileges

#### FreeTokenPrivileges

#### LookupPrivilegesValue

#### EnableAvailablePrivileges

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

#### EnablePrivilege

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

#### EnablePrivilegeEx

- Similar to EnablePrivilege, but uses LUID instead of privilege name.

#### EnablePrivileges

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

#### EnablePrivilegesV

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

#### EnablePrivilegesEx

- Similar to EnablePrivileges, but uses LUIDs instead of privilege names.

#### EnablePrivilegesExV

- Similar to EnablePrivilegeV, but uses LUIDs instead of privilege names.

#### IsPrivilegeEnabled

- Example
  - Determines whether the specified privilege is enabled.

    ```C
    #include <Win32Ex/Security/Privilege.h>

    if (IsPrivilegeEnabled(SE_CHANGE_NOTIFY_NAME, NULL))
    {
    }
    ```

#### IsPrivilegeEnabledEx

- Similar to IsPrivilegeEnabled, but uses LUID instead of privilege name.
- Example
  - Determines whether the specified privilege is enabled.

      ```C
      #include <Win32Ex/Security/Privilege.h>

      if (IsPrivilegeEnabledEx(Win32Ex::Security::SeChangeNotifyPrivilege, NULL))
      {
      }
      ```

#### IsPrivilegesEnabled

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

#### IsPrivilegesEnabledV

- Example
  - Determines whether the change notify and shutdown privileges is enabled.

      ```C
      #include <Win32Ex/Security/Privilege.h>

      if (IsPrivilegesEnabledV(NULL, 2, SE_CHANGE_NOTIFY_NAME, SE_SHUTDOWN_NAME))
      {
      }
      ```

#### IsPrivilegesEnabledEx

- Similar to IsPrivilegesEnabled, but uses LUIDs instead of privilege names.

#### IsPrivilegesEnabledExV

- Similar to IsPrivilegesEnabledV, but uses LUIDs instead of privilege names.

### Variables

#### Privilege LUID variables [*C++ Only*]

##### SeCreateTokenPrivilege

##### SeAssignPrimaryTokenPrivilege

##### SeLockMemoryPrivilege

##### SeIncreaseQuotaPrivilege

##### SeUnsolicitedInputPrivilege

##### SeMachineAccountPrivilege

##### SeTcbPrivilege

##### SeSecurityPrivilege

##### SeTakeOwnershipPrivilege

##### SeLoadDriverPrivilege

##### SeSystemProfilePrivilege

##### SeSystemtimePrivilege

##### SeProfileSingleProcessPrivilege

##### SeIncreaseBasePriorityPrivilege

##### SeCreatePagefilePrivilege

##### SeCreatePermanentPrivilege

##### SeBackupPrivilege

##### SeRestorePrivilege

##### SeShutdownPrivilege

##### SeDebugPrivilege

##### SeAuditPrivilege

##### SeSystemEnvironmentPrivilege

##### SeChangeNotifyPrivilege

##### SeRemoteShutdownPrivilege

##### SeUndockPrivilege

##### SeSyncAgentPrivilege

##### SeEnableDelegationPrivilege

##### SeManageVolumePrivilege

##### SeImpersonatePrivilege

##### SeCreateGlobalPrivilege

##### SeTrustedCredManAccessPrivilege

##### SeRelabelPrivilege

##### SeIncreaseWorkingSetPrivilege

##### SeTimeZonePrivilege

##### SeCreateSymbolicLinkPrivilege

##### SeDelegateSessionUserImpersonatePrivilege

### Classes

#### TokenPrivileges

- Example
  - Adjust a debug and shutdown privileges.

      ```C++
      #include <Win32Ex/System/Privilege.h>
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
      #include <Win32Ex/System/Privilege.hpp>
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
      #include <Win32Ex/System/Privilege.hpp>
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

### Macros

#### Privilege macros

##### SE_MIN_WELL_KNOWN_PRIVILEGE

##### SE_MAX_WELL_KNOWN_PRIVILEGE

##### SE_PROF_SINGLE_PROCESS_NAME_W

##### SE_INC_BASE_PRIORITY_NAME_W

##### SE_CREATE_PAGEFILE_NAME_W

##### SE_CREATE_PERMANENT_NAME_W

##### SE_BACKUP_NAME_W

##### SE_RESTORE_NAME_W

##### SE_SHUTDOWN_NAME_W

##### SE_DEBUG_NAME_W

##### SE_AUDIT_NAME_W

##### SE_SYSTEM_ENVIRONMENT_NAME_W

##### SE_CHANGE_NOTIFY_NAME_W

##### SE_REMOTE_SHUTDOWN_NAME_W

##### SE_UNDOCK_NAME_W

##### SE_SYNC_AGENT_NAME_W

##### SE_ENABLE_DELEGATION_NAME_W

##### SE_MANAGE_VOLUME_NAME_W

##### SE_IMPERSONATE_NAME_W

##### SE_CREATE_GLOBAL_NAME_W

##### SE_TRUSTED_CREDMAN_ACCESS_NAME_W

##### SE_RELABEL_NAME_W

##### SE_INCORKING_SET_NAME_W

##### SE_TIME_ZONE_NAME_W

##### SE_CREATE_SYMBOLIC_LINK_NAME_W

##### SE_DELEGATE_SESSION_USER_IMPERSONATE_NAME_W
