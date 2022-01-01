#include <gtest/gtest.h>

#include <Win32Ex/Result.hpp>

Win32Ex::Result<Win32Ex::String> OkFn()
{
#ifdef _INC__MINGW_H
    return Win32Ex::String("Succeed :-)");
#else
    return "Succeed :-)";
#endif
}

Win32Ex::Result<Win32Ex::String> ErrorFn()
{
    return Win32Ex::Error(ERROR_INVALID_FUNCTION, "ErrorFn - Invalid function :-(");
}

Win32Ex::Result<const Win32Ex::String &> OkFnRef(const Win32Ex::String &Ref)
{
    return Ref;
}

Win32Ex::Result<const Win32Ex::String &> ErrorFnRef()
{
    return Win32Ex::Error(ERROR_INVALID_FUNCTION, "ErrorFnRef - Invalid function :-(");
}

TEST(ResultTest, ValueType)
{
    Win32Ex::Result<Win32Ex::String> result = ErrorFn();
    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.Error().ErrorCode, ERROR_INVALID_FUNCTION);
    EXPECT_STREQ(result.Error().what(), "ErrorFn - Invalid function :-(");
    EXPECT_THROW(result.Get(), Win32Ex::Error);

    result = OkFn();
    EXPECT_TRUE(result.IsOk());
    EXPECT_STREQ(result.Get().c_str(), "Succeed :-)");
}

TEST(ResultTest, RefType)
{
    Win32Ex::Result<const Win32Ex::String &> result = ErrorFnRef();
    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.Error().ErrorCode, ERROR_INVALID_FUNCTION);
    EXPECT_STREQ(result.Error().what(), "ErrorFnRef - Invalid function :-(");
    EXPECT_THROW(result.Get(), Win32Ex::Error);

    Win32Ex::String val = "test";
    result = OkFnRef(val);
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(&result.Get(), &val);
}

TEST(ResultTest, CloneValueType)
{
    Win32Ex::Result<Win32Ex::String> result = OkFn();
    EXPECT_TRUE(result.IsOk());
    EXPECT_FALSE(result.IsErr());
    EXPECT_FALSE(result.IsMoved());
    EXPECT_STREQ(result.Get().c_str(), "Succeed :-)");

    Win32Ex::Result<Win32Ex::String> result2 = result;
    EXPECT_FALSE(result.IsOk());
    EXPECT_FALSE(result.IsErr());
    EXPECT_TRUE(result.IsMoved());
    EXPECT_THROW(result.Get(), Win32Ex::MovedException);
    EXPECT_TRUE(result2.IsOk());
    EXPECT_FALSE(result2.IsErr());
    EXPECT_FALSE(result2.IsMoved());
    EXPECT_STREQ(result2.Get().c_str(), "Succeed :-)");

    result = result2.Clone();
    EXPECT_TRUE(result.IsOk());
    EXPECT_STREQ(result.Get().c_str(), "Succeed :-)");
    EXPECT_TRUE(result2.IsOk());
    EXPECT_STREQ(result2.Get().c_str(), "Succeed :-)");
}

TEST(ResultTest, CloneRefType)
{
    Win32Ex::String val = "test";
    Win32Ex::Result<const Win32Ex::String &> result = OkFnRef(val);
    EXPECT_TRUE(result.IsOk());
    EXPECT_FALSE(result.IsErr());
    EXPECT_FALSE(result.IsMoved());
    EXPECT_EQ(&result.Get(), &val);

    Win32Ex::Result<const Win32Ex::String &> result2 = result;
    EXPECT_FALSE(result.IsOk());
    EXPECT_FALSE(result.IsErr());
    EXPECT_TRUE(result.IsMoved());
    EXPECT_THROW(result.Get(), Win32Ex::MovedException);
    EXPECT_TRUE(result2.IsOk());
    EXPECT_FALSE(result2.IsErr());
    EXPECT_FALSE(result2.IsMoved());
    EXPECT_EQ(&result2.Get(), &val);

    result = result2.Clone();
    EXPECT_TRUE(result.IsOk());
    EXPECT_FALSE(result.IsErr());
    EXPECT_FALSE(result.IsMoved());
    EXPECT_EQ(&result.Get(), &val);
    EXPECT_TRUE(result2.IsOk());
    EXPECT_FALSE(result2.IsErr());
    EXPECT_FALSE(result2.IsMoved());
    EXPECT_EQ(&result2.Get(), &val);
}