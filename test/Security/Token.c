#include <Win32Ex/Security/Token.h>

BOOL LookupTokenC()
{
    HANDLE token = LookupToken(TOKEN_QUERY, NULL);
    if (token)
        CloseHandle(token);
    return token != NULL;
}

BOOL LookupToken2C()
{
    HANDLE token = LookupToken2(TOKEN_QUERY, NULL);
    if (token)
        CloseHandle(token);
    return token != NULL;
}