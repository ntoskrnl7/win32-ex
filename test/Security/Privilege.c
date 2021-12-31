#include <Win32Ex/Security/Privilege.h>

BOOL IsPrivilegeEnabledC(_In_ LPCTSTR PrivilegeName)
{
    return IsPrivilegeEnabled(PrivilegeName, NULL);
}

BOOL IsPrivilegesEnabledC(DWORD NumberOfPrivilegeNames, /*LPCTSTR PrivilegeNames[]*/...)
{
    va_list PrivilegeNames;
    va_start(PrivilegeNames, NumberOfPrivilegeNames);
    return IsPrivilegesEnabled(NumberOfPrivilegeNames, (PCTSTR *)PrivilegeNames, NULL);
}

BOOL EnableAvailablePrivilegesC()
{
    PREVIOUS_TOKEN_PRIVILEGES prev;
    if (!EnableAvailablePrivileges(TRUE, &prev, NULL))
        return FALSE;
    return RevertPrivileges(&prev);
}