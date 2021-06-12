#include <Security\Privilege.h>

BOOL IsPrivilegeEnabledTestC(_In_ LPCTSTR PrivilegeName)
{
    return IsPrivilegeEnabled(PrivilegeName, NULL);
}

BOOL IsPrivilegesEnabledTestC(DWORD NumberOfPrivilegeNames, /*LPCTSTR PrivilegeNames[]*/...)
{
    LPCTSTR *PrivilegeNames;
    va_start((va_list)PrivilegeNames, NumberOfPrivilegeNames);
    return IsPrivilegesEnabled(NumberOfPrivilegeNames, PrivilegeNames, NULL);
}

BOOL EnableAvailablePrivilegesTestC()
{
    PREVIOUS_TOKEN_PRIVILEGES prev;
    if (!EnableAvailablePrivileges(TRUE, &prev, NULL))
        return FALSE;
    return RevertPrivileges(&prev);
}