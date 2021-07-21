#include <System\Object.h>
#include <gtest\gtest.h>

TEST(ObjectTest, MakePermanentObjectTest)
{
    HANDLE handle = CreateMutex(NULL, FALSE, TEXT("Test"));
    if (handle != NULL)
    {
        if (IsUserAdmin())
        {
            EXPECT_FALSE(IsPermanentObject(handle));
            EXPECT_TRUE(IsTemporaryObject(handle));
            EXPECT_TRUE(MakePermanentObject(handle));
            EXPECT_TRUE(IsPermanentObject(handle));
            EXPECT_FALSE(IsTemporaryObject(handle));
        }
        EXPECT_TRUE(MakeTemporaryObject(handle));
        EXPECT_FALSE(IsPermanentObject(handle));
        EXPECT_TRUE(IsTemporaryObject(handle));
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
    EXPECT_TRUE(MakePermanentObjectTestC());
}