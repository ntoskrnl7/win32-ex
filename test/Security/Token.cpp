#include <Win32Ex/Security/Privilege.hpp>
#include <Win32Ex/Security/Token.hpp>

#include <gtest/gtest.h>

using namespace Win32Ex;

TEST(TokenTest, GetTokenInfo)
{
    DWORD returnLength = 0;
    PVOID user = GetTokenInfo(GetCurrentProcessToken(), TokenUser, &returnLength);
    EXPECT_NE(user, (PVOID)NULL);
    EXPECT_NE(returnLength, 0);
    if (user)
        FreeTokenInfo(user);
}

TEST(TokenTest, GetTokenUser)
{
    PTOKEN_USER user = GetTokenUser(GetCurrentProcessToken());
    EXPECT_NE(user, (PTOKEN_USER)NULL);
    if (user)
        FreeTokenUser(user);
}

TEST(TokenTest, GetTokenPrivileges)
{
    PTOKEN_PRIVILEGES priv = GetTokenPrivileges(GetCurrentProcessToken());
    EXPECT_NE(priv, (PTOKEN_PRIVILEGES)NULL);
    if (priv)
        FreeTokenPrivileges(priv);
}

#ifndef __cpp_lambdas
BOOL HasGroupLogonID(HANDLE TokenHandle)
{
    Security::Token token(TokenHandle, false);
    std::vector<SID_AND_ATTRIBUTES> groups = token.Groups();
    for (std::vector<SID_AND_ATTRIBUTES>::const_iterator it = groups.begin(); it != groups.end(); ++it)
    {
        const SID_AND_ATTRIBUTES &group = *it;
        if ((group.Attributes & SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID)
            return TRUE;
    }
    return FALSE;
}
#endif

TEST(TokenTest, LookupToken)
{
#ifdef __cpp_lambdas
    HANDLE token = LookupToken(TOKEN_QUERY, [](HANDLE TokenHandle) -> BOOL {
        Security::Token token(TokenHandle, false);
#if defined(__cpp_range_based_for)
        for (auto &group : token.Groups())
        {
#else
        std::vector<SID_AND_ATTRIBUTES> groups = token.Groups();
        for (std::vector<SID_AND_ATTRIBUTES>::const_iterator it = groups.begin(); it != groups.end(); ++it)
        {
            const SID_AND_ATTRIBUTES &group = *it;
#endif
            if ((group.Attributes & SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID)
                return TRUE;
        }
        return FALSE;
    });
#else
    HANDLE token = LookupToken(TOKEN_QUERY, HasGroupLogonID);
#endif
    EXPECT_NE(token, (HANDLE)NULL);
    if (token)
        CloseHandle(token);
}

TEST(TokenTest, LookupToken2)
{
    HANDLE token = LookupToken2(TOKEN_QUERY, NULL);
    EXPECT_NE(token, (HANDLE)NULL);
    if (token)
        CloseHandle(token);
}

extern "C"
{
    BOOL LookupTokenC();
    BOOL LookupToken2C();
}

TEST(TokenTest, LookupTokenC)
{
    EXPECT_TRUE(LookupTokenC() == TRUE);
}

TEST(TokenTest, LookupToken2C)
{
    EXPECT_TRUE(LookupToken2C() == TRUE);
}

using namespace Win32Ex;
TEST(TokenTest, GetLocalSystemToken)
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
#ifdef __cpp_initializer_lists
                Security::TokenPrivileges priv(
                    Security::FromPrivilegeNames({SE_INCREASE_QUOTA_NAME, SE_ASSIGNPRIMARYTOKEN_NAME}));
#else
                std::vector<LPCTSTR> args;
                args.push_back(SE_INCREASE_QUOTA_NAME);
                args.push_back(SE_ASSIGNPRIMARYTOKEN_NAME);
                Security::TokenPrivileges priv(Security::FromPrivilegeNames(args));
#endif

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
                            EXPECT_TRUE(IsLocalSystemToken(GetCurrentProcessToken()) == TRUE);
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

TEST(TokenTest, TokenAdjustPrivilege)
{
    Security::Token token = Security::Token::Current();
    Security::TokenPrivileges ctx = token.AdjustPrivilege(Security::SeShutdownPrivilege);
    std::vector<LUID_AND_ATTRIBUTES> privs = token.GetPrivileges();
    bool acquired = false;
    for (std::vector<LUID_AND_ATTRIBUTES>::const_iterator it = privs.begin(); it != privs.end(); ++it)
    {
        const LUID_AND_ATTRIBUTES &priv = *it;
        if ((priv.Luid == Security::SeShutdownPrivilege) && (priv.Attributes & SE_PRIVILEGE_ENABLED))
        {
            acquired = true;
            break;
        }
    }
    EXPECT_TRUE(acquired);
    EXPECT_TRUE(token.IsAcquired(Security::SeShutdownPrivilege));

    ctx.Release();

    acquired = false;
    std::vector<LUID_AND_ATTRIBUTES> privs2 = token.GetPrivileges();
    for (std::vector<LUID_AND_ATTRIBUTES>::const_iterator it = privs2.begin(); it != privs2.end(); ++it)
    {
        const LUID_AND_ATTRIBUTES &priv = *it;
        if ((priv.Luid == Security::SeShutdownPrivilege) && (priv.Attributes & SE_PRIVILEGE_ENABLED))
        {
            acquired = true;
            break;
        }
    };
    EXPECT_FALSE(acquired);
    EXPECT_FALSE(token.IsAcquired(Security::SeShutdownPrivilege));
}

TEST(TokenTest, TokenAdjustPrivileges)
{
    Security::Token token = Security::Token::Current();
#if defined(_cpp_initializer_lists)
    Security::TokenPrivileges ctx =
        token.AdjustPrivileges({Security::SeShutdownPrivilege, Security::SeTimeZonePrivilege});
#else
    std::vector<LUID> args;
    args.push_back(Security::SeShutdownPrivilege);
    args.push_back(Security::SeTimeZonePrivilege);
    Security::TokenPrivileges ctx = token.AdjustPrivileges(args);
#endif
    std::vector<LUID_AND_ATTRIBUTES> privs = token.GetPrivileges();
    bool acquired = false;
    bool acquired1 = false;
    for (std::vector<LUID_AND_ATTRIBUTES>::const_iterator it = privs.begin(); it != privs.end(); ++it)
    {
        const LUID_AND_ATTRIBUTES &priv = *it;
        if (priv.Luid == Security::SeShutdownPrivilege)
        {
            acquired = (priv.Attributes & SE_PRIVILEGE_ENABLED) == SE_PRIVILEGE_ENABLED;
        }
        if (priv.Luid == Security::SeTimeZonePrivilege)
        {
            acquired1 = (priv.Attributes & SE_PRIVILEGE_ENABLED) == SE_PRIVILEGE_ENABLED;
        }
    }
    EXPECT_TRUE(acquired);
    EXPECT_TRUE(acquired1);
#if defined(_cpp_initializer_lists)
    EXPECT_TRUE(token.IsAcquired({Security::SeShutdownPrivilege, Security::SeTimeZonePrivilege}));
#else
    EXPECT_TRUE(token.IsAcquired(args));
#endif

    ctx.Release();
    acquired = false;
    acquired1 = false;
    std::vector<LUID_AND_ATTRIBUTES> privs2 = token.GetPrivileges();
    for (std::vector<LUID_AND_ATTRIBUTES>::const_iterator it = privs2.begin(); it != privs2.end(); ++it)
    {
        const LUID_AND_ATTRIBUTES &priv = *it;
        if (priv.Luid == Security::SeShutdownPrivilege)
        {
            acquired = (priv.Attributes & SE_PRIVILEGE_ENABLED) == SE_PRIVILEGE_ENABLED;
        }
        if (priv.Luid == Security::SeTimeZonePrivilege)
        {
            acquired1 = (priv.Attributes & SE_PRIVILEGE_ENABLED) == SE_PRIVILEGE_ENABLED;
        }
    };
    EXPECT_FALSE(acquired);
    EXPECT_FALSE(acquired1);
#if defined(_cpp_initializer_lists)
    EXPECT_FALSE(token.IsAcquired({Security::SeShutdownPrivilege, Security::SeTimeZonePrivilege}));
#else
    EXPECT_FALSE(token.IsAcquired(args));
#endif
}

#ifndef __cpp_lambdas
BOOL HasCreatePermanentPrivilege(DWORD /*ProcessId*/, HANDLE TokenHandle)
{
    PRIVILEGE_SET privilegeSet;
    privilegeSet.PrivilegeCount = 1;
    privilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
    privilegeSet.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
    privilegeSet.Privilege[0].Luid = Security::SeCreatePermanentPrivilege;
    BOOL result = FALSE;
    return PrivilegeCheck(TokenHandle, &privilegeSet, &result) && result;
}

BOOL HasChangeNotifyPrivilege(DWORD /*ProcessId*/, HANDLE TokenHandle)
{
    PRIVILEGE_SET privilegeSet;
    privilegeSet.PrivilegeCount = 1;
    privilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
    privilegeSet.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
    privilegeSet.Privilege[0].Luid = Security::SeChangeNotifyPrivilege;
    BOOL result = FALSE;
    return PrivilegeCheck(TokenHandle, &privilegeSet, &result) && result;
}
#endif

TEST(TokenTest, NewTokenObjectByHandle)
{
    HANDLE tokenHandle;
    if (OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &tokenHandle))
    {
        Security::Token token(tokenHandle);
        EXPECT_EQ(token.GetPrivileges().size(), Security::Token::Current().GetPrivileges().size());

        EXPECT_TRUE(token.IsValid());

        Security::Token token2 = token;
        EXPECT_FALSE(token.IsValid());
        EXPECT_TRUE(token2.IsValid());

        EXPECT_NE(token.GetPrivileges().size(), Security::Token::Current().GetPrivileges().size());
        EXPECT_EQ(token2.GetPrivileges().size(), Security::Token::Current().GetPrivileges().size());
    }
}

TEST(TokenTest, NewTokenObjectByCondition)
{
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
#ifdef __cpp_lambdas
        Security::Token token([&privilegeSet](DWORD /*ProcessId*/, HANDLE TokenHandle) -> BOOL {
            BOOL result = FALSE;
            return PrivilegeCheck(TokenHandle, &privilegeSet, &result) && result;
        });
#else
        Security::Token token((std::function<BOOL(DWORD, HANDLE)>)HasCreatePermanentPrivilege);
#endif
        EXPECT_TRUE(token.IsValid());

        Security::Token token2 = token;
        EXPECT_FALSE(token.IsValid());
        EXPECT_TRUE(token2.IsValid());
    }
    else
    {
        privilegeSet.Privilege[0].Luid = Security::SeChangeNotifyPrivilege;
#ifdef __cpp_lambdas
        Security::Token token([&privilegeSet](DWORD /*ProcessId*/, HANDLE TokenHandle) -> BOOL {
            BOOL result = FALSE;
            return PrivilegeCheck(TokenHandle, &privilegeSet, &result) && result;
        });
#else
        Security::Token token((std::function<BOOL(DWORD, HANDLE)>)HasChangeNotifyPrivilege);
#endif
        EXPECT_TRUE(token.IsValid());

        Security::Token token2 = token;
        EXPECT_FALSE(token.IsValid());
        EXPECT_TRUE(token2.IsValid());
    }
}