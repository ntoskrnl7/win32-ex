#include <gtest/gtest.h>

#include <Win32Ex/Internal/misc.hpp>

void TestFn(Win32Ex::Optional<Win32Ex::TString> arg0, Win32Ex::Optional<Win32Ex::TString> arg1 = Win32Ex::None())
{
    if (arg0.IsSome())
    {
        Win32Ex::TString val = arg0;
    }
    if (arg1.IsSome())
    {
        Win32Ex::TString val = arg1;
    }
}
#ifdef UNICODE
void TestFn(Win32Ex::Optional<Win32Ex::String> arg0, Win32Ex::Optional<Win32Ex::String> arg1 = Win32Ex::None())
{
    if (arg0.IsSome())
    {
        Win32Ex::String val = arg0;
    }
    if (arg1.IsSome())
    {
        Win32Ex::String val = arg1;
    }
}
#else
void TestFn(Win32Ex::Optional<Win32Ex::WString> arg0, Win32Ex::Optional<Win32Ex::WString> arg1 = Win32Ex::None())
{
    if (arg0.IsSome())
    {
        Win32Ex::WString val = arg0;
    }
    if (arg1.IsSome())
    {
        Win32Ex::WString val = arg1;
    }
}
#endif

using namespace Win32Ex;

DWORD TestFn(Optional<int> arg0 = None(), Optional<int> arg1 = None(), Optional<int> arg2 = None())
{
    DWORD flags = 0x00000000;
    if (arg0.IsSome())
    {
        int val = arg0;
        std::cout << "arg0.IsSome() : " << val << '\n';
        flags |= 0x00000001;
    }
    if (arg1.IsSome())
    {
        int val = arg1;
        std::cout << "arg1.IsSome() : " << val << '\n';
        flags |= 0x00000002;
    }
    if (arg2.IsSome())
    {
        int val = arg2;
        std::cout << "arg2.IsSome() : " << val << '\n';
        flags |= 0x00000004;
    }

    if (arg0.IsSome() || arg1.IsSome() || arg2.IsSome())
    {
        std::cout << "arg0.IsSome() || arg1.IsSome() || arg2.IsSome()\n";
        flags |= 0x00000080;
    }

    if (arg0.IsSome() && arg1.IsSome() && arg2.IsSome())
    {
        std::cout << "arg0.IsSome() && arg1.IsSome() && arg2.IsSome()\n";
        flags |= 0x00000100;
    }
    if (arg0.IsNone() && arg1.IsNone() && arg2.IsNone())
    {
        std::cout << "arg0.IsNone() && arg1.IsNone() && arg2.IsNone()\n";
        flags |= 0x00000200;
    }

#if defined(__cpp_variadic_templates)
    if (IsSome(arg0, arg1, arg2))
    {
        std::cout << "IsSome(arg0, arg1, arg2)\n";
        flags |= 0x10000000;
    }

    if (IsAny(arg0, arg1, arg2))
    {
        std::cout << "IsAny(arg0, arg1, arg2)\n";
        flags |= 0x20000000;
    }

    if (IsAll(arg0, arg1, arg2))
    {
        std::cout << "IsAll(arg0, arg1, arg2)\n";
        flags |= 0x40000000;
    }

    if (IsNone(arg0, arg1, arg2))
    {
        std::cout << "IsNone(arg0, arg1, arg2)\n";
        flags |= 0x80000000;
    }
#endif
    return flags;
}

TEST(OptionalTest, Basic)
{
#if defined(__cpp_variadic_templates)
    const DWORD validFlags = 0xFFFFFFFF;
#else
    const DWORD validFlags = 0x0FFFFFFF;
#endif
    std::cout << "TestFn()\n";
    EXPECT_EQ(TestFn(), 0x80000200 & validFlags);

    std::cout << "TestFn(1)\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(1), 0x30000081 & validFlags);

    std::cout << "TestFn(1, None())\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(1, None()), 0x30000081 & validFlags);

    std::cout << "TestFn(1, None(), None())\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(1, None(), None()), 0x30000081 & validFlags);

    std::cout << "TestFn(None(), 2)\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(None(), 2), 0x30000082 & validFlags);

    std::cout << "TestFn(None(), 2, None())\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(None(), 2, None()), 0x30000082 & validFlags);

    std::cout << "TestFn(None(), None(), 3)\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(None(), None(), 3), 0x30000084 & validFlags);

    std::cout << "TestFn(1, 2)\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(1, 2), 0x30000083 & validFlags);

    std::cout << "TestFn(1, 2, None())\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(1, 2, None()), 0x30000083 & validFlags);

    std::cout << "TestFn(None(), 2, 3)\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(None(), 2, 3), 0x30000086 & validFlags);

    std::cout << "TestFn(1, None(), 3)\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(1, None(), 3), 0x30000085 & validFlags);

    std::cout << "TestFn(1, 2, 3)\n-------------------------------\n\n";
    EXPECT_EQ(TestFn(1, 2, 3), 0x70000187 & validFlags);
}

TEST(OptionalTest, NoThrow)
{
    EXPECT_NO_THROW({
        TestFn();
        TestFn(1);
        TestFn(1, None());
        TestFn(1, None(), None());
        TestFn(None(), 2);
        TestFn(1, 2, None());
        TestFn(1, 2);
        TestFn(1, 2, None());
        TestFn(None(), None(), 3);
        TestFn(None(), 2, 3);
        TestFn(1, 2, 3);

        TestFn("test");
        TestFn("test", "test2");
        TestFn(L"test");
        TestFn(L"test", L"test2");
        TestFn(TEXT("test"));
        TestFn(TEXT("test"), TEXT("test2"));

        TestFn(None(), TEXT("test2"));
    });
}

TEST(OptionalTest, Throw)
{
    EXPECT_THROW(TestFn(TEXT("test"), NULL), NullException);
#if !defined(_MSC_VER) || _MSC_VER >= 1600
    EXPECT_THROW(TestFn(TEXT("test"), nullptr), NullException);
#endif
    EXPECT_THROW(
        {
            Optional<WString> value = NULL;
            WString val = value;
        },
        NullException);
    EXPECT_THROW(
        {
            Optional<WString> value;
            WString val = value;
        },
        Exception);
    EXPECT_THROW(
        {
            Optional<int> value;
            int val = value;
        },
        Exception);
}