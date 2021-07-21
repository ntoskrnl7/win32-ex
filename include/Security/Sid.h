﻿#pragma once

#define WIN32_LEAN_AND_MEAN
#include <AclAPI.h>

DECLSPEC_SELECTANY SID EveryoneSid = {SID_REVISION, 1, SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID};

#ifdef SECURITY_LOCAL_LOGON_RID
DECLSPEC_SELECTANY SID ConsoleLogonSid = {SID_REVISION, 1, SECURITY_LOCAL_SID_AUTHORITY, SECURITY_LOCAL_LOGON_RID};
#endif
DECLSPEC_SELECTANY SID LocalSid = {SID_REVISION, 1, SECURITY_LOCAL_SID_AUTHORITY, SECURITY_LOCAL_RID};

DECLSPEC_SELECTANY SID CreatorOwnerSid = {SID_REVISION, 1, SECURITY_CREATOR_SID_AUTHORITY, SECURITY_CREATOR_OWNER_RID};
DECLSPEC_SELECTANY SID CreatorGroupSid = {SID_REVISION, 1, SECURITY_CREATOR_SID_AUTHORITY, SECURITY_CREATOR_GROUP_RID};

DECLSPEC_SELECTANY SID ServiceSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_SERVICE_RID};
DECLSPEC_SELECTANY SID LocalSystemSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_LOCAL_SYSTEM_RID};
DECLSPEC_SELECTANY SID LocalServiceSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_LOCAL_SERVICE_RID};
DECLSPEC_SELECTANY SID NetworkServiceSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_NETWORK_SERVICE_RID};