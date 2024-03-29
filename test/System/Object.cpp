﻿#include <Win32Ex/System/Object.h>
#include <gtest/gtest.h>

TEST(ObjectTest, MakePermanentObject)
{
    HANDLE handle = CreateMutex(NULL, FALSE, TEXT("Test"));
    if (handle != NULL)
    {
        if (IsUserAdmin())
        {
            EXPECT_FALSE(IsPermanentObject(handle) == TRUE);
            EXPECT_TRUE(IsTemporaryObject(handle) == TRUE);
            EXPECT_TRUE(MakePermanentObject(handle) == TRUE);
            EXPECT_TRUE(IsPermanentObject(handle) == TRUE);
            EXPECT_FALSE(IsTemporaryObject(handle) == TRUE);
        }
        EXPECT_TRUE(MakeTemporaryObject(handle) == TRUE);
        EXPECT_FALSE(IsPermanentObject(handle) == TRUE);
        EXPECT_TRUE(IsTemporaryObject(handle) == TRUE);
        CloseHandle(handle);
    }
}

/**
 * @brief Test a C Code
 *
 */
extern "C"
{
    extern BOOL MakePermanentObjectC();
}

TEST(ObjectTest, MakePermanentObjectC)
{
    EXPECT_TRUE(MakePermanentObjectC() == TRUE);
}