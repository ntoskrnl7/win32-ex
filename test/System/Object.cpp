#include <System\Object.h>
#include <gtest\gtest.h>

TEST(ObjectTest, MakePermanentObjectTest)
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
    extern BOOL MakePermanentObjectTestC();
}

TEST(ObjectTest, MakePermanentObjectTestC)
{
    EXPECT_TRUE(MakePermanentObjectTestC() == TRUE);
}