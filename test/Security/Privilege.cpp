#include <Win32Ex/Security/Privilege.hpp>

#include <gtest/gtest.h>

TEST(PrivilegeTest, EnableAvailablePrivileges)
{
    PREVIOUS_TOKEN_PRIVILEGES prev;
    EXPECT_TRUE(EnableAvailablePrivileges(TRUE, &prev, NULL) == TRUE);
    EXPECT_TRUE(RevertPrivileges(&prev) == TRUE);
}

TEST(PrivilegeTest, IsPrivilegeEnabled)
{
	EXPECT_TRUE(IsPrivilegeEnabled(SE_CHANGE_NOTIFY_NAME, NULL) == TRUE);
}

TEST(PrivilegeTest, IsPrivilegeEnabledEx)
{
	EXPECT_TRUE(IsPrivilegeEnabledEx(Win32Ex::Security::SeChangeNotifyPrivilege, NULL) == TRUE);
}


TEST(PrivilegeTest, EnablePrivileges)
{
	PREVIOUS_TOKEN_PRIVILEGES prev;
	LPCTSTR privileges[] = {
		SE_CHANGE_NOTIFY_NAME,
		SE_SHUTDOWN_NAME
	};

	EXPECT_TRUE(EnablePrivileges(TRUE, 2, privileges, &prev, NULL) == TRUE);
    EXPECT_TRUE(RevertPrivileges(&prev) == TRUE);
}

TEST(PrivilegeTest, EnablePrivilegesV)
{
    PREVIOUS_TOKEN_PRIVILEGES prev;
    EXPECT_TRUE(EnablePrivilegesV(TRUE, &prev, NULL, 2, SE_CHANGE_NOTIFY_NAME, SE_SHUTDOWN_NAME) == TRUE);
    EXPECT_TRUE(IsPrivilegesEnabledV(NULL, 2, SE_CHANGE_NOTIFY_NAME, SE_SHUTDOWN_NAME) == TRUE);
    EXPECT_TRUE(RevertPrivileges(&prev) == TRUE);
}

TEST(PrivilegeTest, EnablePrivilegesExV)
{
    PREVIOUS_TOKEN_PRIVILEGES prev;
    EXPECT_TRUE(EnablePrivilegesV(TRUE, &prev, NULL, 2, SE_CHANGE_NOTIFY_NAME, SE_SHUTDOWN_NAME) == TRUE);
    EXPECT_TRUE(IsPrivilegesEnabledV(NULL, 2, SE_CHANGE_NOTIFY_NAME, SE_SHUTDOWN_NAME) == TRUE);
    EXPECT_TRUE(RevertPrivileges(&prev) == TRUE);
}

using namespace Win32Ex;
TEST(PrivilegeTest, PrivilegeClass)
{
    Security::TokenPrivileges priv(Security::SeIncreaseWorkingSetPrivilege);
    EXPECT_TRUE(priv.IsAcquired());
    EXPECT_EQ(priv.AcquiredPrivileges().size(), 1);

    {
#ifdef __cpp_initializer_lists
        Security::TokenPrivileges priv(
            {Security::SeDebugPrivilege, Security::SeShutdownPrivilege, Security::SeChangeNotifyPrivilege});
#else
        std::vector<LUID> args;
        args.push_back(Security::SeDebugPrivilege);
        args.push_back(Security::SeShutdownPrivilege);
        args.push_back(Security::SeChangeNotifyPrivilege);
        Security::TokenPrivileges priv(args);
#endif
        if (IsUserAdmin())
        {
            EXPECT_TRUE(priv.IsAcquired());
            EXPECT_EQ(priv.AcquiredPrivileges().size(), 3);
        }
        else
        {
            EXPECT_FALSE(priv.IsAcquired());
            EXPECT_EQ(priv.AcquiredPrivileges().size(), 2);
        }
    }

    Security::TokenPrivileges priv2(SE_INC_WORKING_SET_NAME);
    EXPECT_TRUE(priv2.IsAcquired());
    EXPECT_EQ(priv2.AcquiredPrivileges().size(), 1);
    {

#ifdef __cpp_initializer_lists
        Security::TokenPrivileges priv(
            Security::FromPrivilegeNames({SE_DEBUG_NAME, SE_SHUTDOWN_NAME, SE_CHANGE_NOTIFY_NAME}));
#else
        std::vector<LPCTSTR> args;
        args.push_back(SE_DEBUG_NAME);
        args.push_back(SE_SHUTDOWN_NAME);
        args.push_back(SE_CHANGE_NOTIFY_NAME);
        Security::TokenPrivileges priv(Security::FromPrivilegeNames(args));
#endif
        if (IsUserAdmin())
        {
            EXPECT_TRUE(priv.IsAcquired());
            EXPECT_EQ(priv.AcquiredPrivileges().size(), 3);
        }
        else
        {
            EXPECT_FALSE(priv.IsAcquired());
            EXPECT_EQ(priv.AcquiredPrivileges().size(), 2);
        }
    }
    {

#ifdef __cpp_initializer_lists
        Security::TokenPrivileges priv({SE_DEBUG_NAME, SE_SHUTDOWN_NAME, SE_CHANGE_NOTIFY_NAME});
#else
        std::vector<PCTSTR> args;
        args.push_back(SE_DEBUG_NAME);
        args.push_back(SE_SHUTDOWN_NAME);
        args.push_back(SE_CHANGE_NOTIFY_NAME);
        Security::TokenPrivileges priv(args);
#endif
        if (IsUserAdmin())
        {
            EXPECT_TRUE(priv.IsAcquired());
            EXPECT_EQ(priv.AcquiredPrivileges().size(), 3);
        }
        else
        {
            EXPECT_FALSE(priv.IsAcquired());
            EXPECT_EQ(priv.AcquiredPrivileges().size(), 2);
        }
    }
    {
        Security::TokenPrivileges priv(3, SE_DEBUG_NAME, SE_SHUTDOWN_NAME, SE_CHANGE_NOTIFY_NAME);
        if (IsUserAdmin())
        {
            EXPECT_TRUE(priv.IsAcquired());
            EXPECT_EQ(priv.AcquiredPrivileges().size(), 3);
        }
        else
        {
            EXPECT_FALSE(priv.IsAcquired());
            EXPECT_EQ(priv.AcquiredPrivileges().size(), 2);
        }
    }
}

/**
 * @brief Test a C Code
 *
 */
extern "C"
{
    extern BOOL EnableAvailablePrivilegesC();
    extern BOOL IsPrivilegeEnabledC(LPCTSTR PrivilegeNames);
    extern BOOL IsPrivilegesEnabledC(DWORD NumberOfPrivilegeNames, /*LPCTSTR PrivilegeNames[]*/...);
}

TEST(PrivilegeTest, EnableAvailablePrivilegesC)
{
    EXPECT_TRUE(EnableAvailablePrivilegesC() == TRUE);
}

TEST(PrivilegeTest, IsPrivilegeEnabledC)
{
    PREVIOUS_TOKEN_PRIVILEGES prev;
    if (EnablePrivilege(TRUE, SE_SHUTDOWN_NAME, &prev, NULL))
    {
        EXPECT_TRUE(IsPrivilegeEnabledC(SE_SHUTDOWN_NAME) == TRUE);
        RevertPrivileges(&prev);
    }
}

TEST(PrivilegeTest, IsPrivilegesEnabledC)
{
    PREVIOUS_TOKEN_PRIVILEGES prev;
    if (EnablePrivilegesV(TRUE, &prev, NULL, 2, SE_SHUTDOWN_NAME, SE_CHANGE_NOTIFY_NAME))
    {
        EXPECT_TRUE(IsPrivilegesEnabledC(2, SE_SHUTDOWN_NAME, SE_CHANGE_NOTIFY_NAME) == TRUE);
        RevertPrivileges(&prev);
    }
}