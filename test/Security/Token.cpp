#include <Security\Privilege.hpp>
#include <Security\Token.hpp>

#include <gtest\gtest.h>

using namespace Win32Ex;

TEST(TokenTest, GetTokenInfoTest)
{
    DWORD returnLength = 0;
    PVOID user = GetTokenInfo(GetCurrentProcessToken(), TokenUser, &returnLength);
    EXPECT_NE(user, (PVOID)NULL);
    EXPECT_NE(returnLength, 0);
    if (user)
        FreeTokenInfo(user);
}

TEST(TokenTest, GetTokenUserTest)
{
    PTOKEN_USER user = GetTokenUser(GetCurrentProcessToken());
    EXPECT_NE(user, (PTOKEN_USER)NULL);
    if (user)
        FreeTokenUser(user);
}

TEST(TokenTest, GetTokenPrivilegesTest)
{
    PTOKEN_PRIVILEGES priv = GetTokenPrivileges(GetCurrentProcessToken());
    EXPECT_NE(priv, (PTOKEN_PRIVILEGES)NULL);
    if (priv)
        FreeTokenPrivileges(priv);
}

TEST(TokenTest, LookupTokenTest)
{
    HANDLE token = LookupToken(TOKEN_QUERY, [](HANDLE TokenHandle) {
        Security::Token token(TokenHandle, false);
        for (auto group : token.GetGroups())
            if ((group.Attributes & SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID)
                return TRUE;
        return FALSE;
    });
    EXPECT_NE(token, (HANDLE)NULL);
    if (token)
        CloseHandle(token);
}

TEST(TokenTest, LookupToken2Test)
{
    HANDLE token = LookupToken2(TOKEN_QUERY, NULL);
    EXPECT_NE(token, (HANDLE)NULL);
    if (token)
        CloseHandle(token);
}

extern "C"
{
    BOOL LookupTokenTestC();
    BOOL LookupToken2TestC();
}

TEST(TokenTest, LookupTokenTestC)
{
    EXPECT_TRUE(LookupTokenTestC());
}

TEST(TokenTest, LookupToken2TestC)
{
    EXPECT_TRUE(LookupToken2TestC());
}

using namespace Win32Ex;
TEST(TokenTest, GetLocalSystemTokenTest)
{
    if (IsUserAdmin())
    {
        Security::TokenPrivileges priv(Security::SeDebugPrivilege);
        if (!priv.IsAcquired())
            return;
        HANDLE token = GetLocalSystemToken(TOKEN_QUERY | TOKEN_READ | TOKEN_IMPERSONATE | TOKEN_QUERY_SOURCE |
                                           TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_EXECUTE);
        EXPECT_NE(token, (HANDLE)NULL);

        if (token)
        {
            HANDLE newToken;
            if (DuplicateTokenEx(token, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &newToken))
            {
                Security::TokenPrivileges priv(
                    Security::FromPrivilegeNames({SE_INCREASE_QUOTA_NAME, SE_ASSIGNPRIMARYTOKEN_NAME}));
                if (priv.IsAcquired() && ImpersonateLoggedOnUser(newToken))
                {
                    DWORD origSessionId = 0;
                    DWORD returnLength = 0;
                    if (GetTokenInformation(newToken, TokenSessionId, &origSessionId, sizeof(origSessionId),
                                            &returnLength))
                    {
                        DWORD sessionId = 0;
                        if (SetTokenInformation(newToken, TokenSessionId, &sessionId, sizeof(sessionId)))
                        {
                            EXPECT_TRUE(IsLocalSystemToken(GetCurrentProcessToken()));
                            RevertToSelf();
                            SetTokenInformation(newToken, TokenSessionId, &origSessionId, sizeof(origSessionId));
                        }
                    }
                }
                CloseHandle(newToken);
            }

            CloseHandle(token);
        }
    }
}

TEST(TokenTest, TokenClassTest)
{
    Security::Token token = Security::Token::Current();
    auto ctx = token.AdjustPrivileges({Security::SeShutdownPrivilege});
    auto privs = token.GetPrivileges();

    bool acquired = false;
    for (auto priv : privs)
    {
        if ((priv.Luid == Security::SeShutdownPrivilege) && (priv.Attributes & SE_PRIVILEGE_ENABLED))
        {
            acquired = true;
            break;
        }
    }
    EXPECT_TRUE(acquired);

    ctx.Release();
    auto privs2 = token.GetPrivileges();

    acquired = false;
    for (auto priv : privs2)
    {
        if ((priv.Luid == Security::SeShutdownPrivilege) && (priv.Attributes & SE_PRIVILEGE_ENABLED))
        {
            acquired = true;
            break;
        }
    }
    EXPECT_FALSE(acquired);

    PRIVILEGE_SET privilegeSet;
    privilegeSet.PrivilegeCount = 1;
    privilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
    privilegeSet.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (IsUserAdmin())
    {
        Security::TokenPrivileges priv(Security::SeDebugPrivilege);
        if (!priv.IsAcquired())
            return;
        privilegeSet.Privilege[0].Luid = Security::SeCreatePermanentPrivilege;
        Security::Token token([&privilegeSet](DWORD /*ProcessId*/, HANDLE TokenHandle) {
            BOOL result = FALSE;
            return PrivilegeCheck(TokenHandle, &privilegeSet, &result) && result;
        });
        EXPECT_TRUE(token.IsValid());
        Security::Token token2 = std::move(token);
        EXPECT_FALSE(token.IsValid());
        EXPECT_TRUE(token2.IsValid());
    }
    else
    {
        privilegeSet.Privilege[0].Luid = Security::SeChangeNotifyPrivilege;
        Security::Token token([&privilegeSet](DWORD /*ProcessId*/, HANDLE TokenHandle) {
            BOOL result = FALSE;
            return PrivilegeCheck(TokenHandle, &privilegeSet, &result) && result;
        });
        EXPECT_TRUE(token.IsValid());
        Security::Token token2 = std::move(token);
        EXPECT_FALSE(token.IsValid());
        EXPECT_TRUE(token2.IsValid());
    }
}