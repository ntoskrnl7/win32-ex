#include <Security/Token.h>

BOOL LookupTokenTestC()
{
    HANDLE token = LookupToken(TOKEN_QUERY, NULL);
    if (token)
        CloseHandle(token);
    return token != NULL;
}

BOOL LookupToken2TestC()
{
    HANDLE token = LookupToken2(TOKEN_QUERY, NULL);
    if (token)
        CloseHandle(token);
    return token != NULL;
}