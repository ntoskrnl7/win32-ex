#include <gtest/gtest.h>

#include <Win32Ex/Optional.hpp>

void TestFn(Win32Ex::Optional<Win32Ex::StringT> arg0, Win32Ex::Optional<Win32Ex::StringT> arg1 = Win32Ex::None())
{
    if (arg0.IsSome())
    {
        Win32Ex::StringT val = arg0;
        PCTSTR val2 = arg0.Get();
    }
    if (arg1.IsSome())
    {
        Win32Ex::StringT val = arg1;
        PCTSTR val2 = arg1.Get();
    }
}
#ifdef UNICODE
void TestFn(Win32Ex::Optional<Win32Ex::String> arg0, Win32Ex::Optional<Win32Ex::String> arg1 = Win32Ex::None())
{
    if (arg0.IsSome())
    {
        Win32Ex::String val = arg0;
        PCSTR val2 = arg0.Get();
    }
    if (arg1.IsSome())
    {
        Win32Ex::String val = arg1;
        PCSTR val2 = arg1.Get();
    }
}
#else
void TestFn(Win32Ex::Optional<Win32Ex::StringW> arg0, Win32Ex::Optional<Win32Ex::StringW> arg1 = Win32Ex::None())
{
    if (arg0.IsSome())
    {
        Win32Ex::StringW val = arg0;
        PCWSTR val2 = arg0.Get();
    }
    if (arg1.IsSome())
    {
        Win32Ex::StringW val = arg1;
        PCWSTR val2 = arg1.Get();
    }
}
#endif

using namespace Win32Ex;

DWORD TestFn(const Optional<int> &arg0 = None(), const Optional<int> &arg1 = None(), const Optional<int> &arg2 = None())
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

DWORD TestFn2(const Optional<int> &arg0 = None(), const Optional<int> &arg1 = None(),
              const Optional<int> &arg2 = None())
{
    return TestFn(arg0, arg1, arg2);
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
            Optional<StringW> value = NULL;
            StringW val = value;
        },
        NullException);
    EXPECT_THROW(
        {
            Optional<StringW> value;
            StringW val = value;
        },
        Exception);
    EXPECT_THROW(
        {
            Optional<int> value;
            int val = value;
        },
        Exception);
}

TEST(OptionalTest, OptionalConstString)
{
    String str = "test";

    Optional<const String> optConstStr = str; // copy

    // String &strRef = optConstStr;       // complie error

    const String &constStrRef = optConstStr;
    // EXPECT_NE(str.c_str(), constStrRef.c_str()); // Failed at MSYS
    EXPECT_STREQ(str.c_str(), constStrRef.c_str());

    String strDup = optConstStr;
    EXPECT_STREQ(str.c_str(), strDup.c_str());

    PCSTR pcstr = optConstStr.Get();
    // EXPECT_NE(str.c_str(), pcstr); // Failed at MSYS
    EXPECT_STREQ(str.c_str(), pcstr);

    // EXPECT_EQ(str.c_str(), optConstStr->c_str()); // complie error

    Optional<const String> optConstStr2 = optConstStr; // move
    EXPECT_STREQ(optConstStr2.Get(), str.c_str());
    EXPECT_TRUE(optConstStr.IsNone());

    optConstStr = NULL;
    EXPECT_THROW(const String &constStrRef = optConstStr, NullException);
    EXPECT_THROW(strDup = optConstStr, NullException);
    EXPECT_NO_THROW(pcstr = optConstStr.Get());
    EXPECT_EQ(pcstr, (PCSTR)NULL);
}

TEST(OptionalTest, OptionalString)
{
    String str = "test";

    Optional<String> optStr = str; // copy

    String &strRef = optStr;
    EXPECT_NE(&str, &strRef);
    EXPECT_EQ(str, strRef);
    strRef = "test2";
    EXPECT_EQ((String)optStr, strRef);
    strRef = str;
    EXPECT_EQ((String)optStr, str);

    const String &constStrRef = optStr;
    // EXPECT_NE(str.c_str(), constStrRef.c_str()); // Failed at MSYS
    EXPECT_STREQ(str.c_str(), constStrRef.c_str());

    String strDup = optStr;
    EXPECT_STREQ(str.c_str(), strDup.c_str());

    PCSTR pcstr = optStr.Get();
    // EXPECT_NE(str.c_str(), pcstr); // Failed at MSYS
    EXPECT_STREQ(str.c_str(), pcstr);

    // EXPECT_EQ(str.c_str(), optStr->c_str()); // complie error

    Optional<String> optStr2 = optStr; // move
    EXPECT_STREQ(optStr2.Get(), str.c_str());
    EXPECT_TRUE(optStr.IsNone());

    optStr = NULL;
    EXPECT_THROW(const String &constStrRef = optStr, NullException);
    EXPECT_THROW(strDup = optStr, NullException);
    EXPECT_NO_THROW(pcstr = optStr.Get());
    EXPECT_EQ(pcstr, (PCSTR)NULL);
}

TEST(OptionalTest, OptionalMove)
{
    Optional<const String &> optConstStrRef = "test";
    Optional<String> optStr = optConstStrRef; // move
    EXPECT_STREQ(optStr.Get(), "test");
    EXPECT_TRUE(optConstStrRef.IsNone());

    optConstStrRef = optStr; // move
    EXPECT_STREQ(optConstStrRef.Get(), "test");
    EXPECT_TRUE(optStr.IsNone());

    const Optional<const String &> constOptConstStrRef = "test";
    const Optional<String> constOptStr = constOptConstStrRef; // copy
    optStr = constOptConstStrRef;                             // copy

    EXPECT_STREQ(constOptConstStrRef.Get(), "test");
    EXPECT_STREQ(constOptStr.Get(), "test");
    EXPECT_STREQ(optStr.Get(), "test");
}

TEST(OptionalTest, OptionalConstStringRef)
{
    String str = "test";

    Optional<const String &> optConstStrRef = str; // const ref

    // String &strRef = optConstStrRef;               // complie error

    const String &constStrRef = optConstStrRef;
    EXPECT_EQ(&str, &constStrRef);

    String strDup = optConstStrRef;

    EXPECT_STREQ(str.c_str(), strDup.c_str());

    PCSTR pcstr = optConstStrRef.Get();
    EXPECT_EQ(str.c_str(), pcstr);

    // EXPECT_EQ(str.c_str(), optConstStrRef->c_str()); // complie error

    Optional<const String &> optConstStrRef2 = optConstStrRef; // move
    EXPECT_STREQ(optConstStrRef2.Get(), str.c_str());
    EXPECT_TRUE(optConstStrRef.IsNone());

    optConstStrRef = NULL;
    EXPECT_THROW(const String &constStrRef = optConstStrRef, NullException);
    EXPECT_THROW(strDup = optConstStrRef, NullException);
    EXPECT_NO_THROW(pcstr = optConstStrRef.Get());
    EXPECT_EQ(pcstr, (PCSTR)NULL);
}

TEST(OptionalTest, OptionalStringRef)
{
    String str = "test";

    Optional<String &> optStrRef = str; // mut ref
    String &strRef = optStrRef;
    EXPECT_EQ(&str, &strRef);

    const String &constStrRef = optStrRef;
    EXPECT_EQ(str.c_str(), constStrRef.c_str());

    String strDup = optStrRef;
    EXPECT_STREQ(str.c_str(), strDup.c_str());

    // PCSTR pcstr = optStrRef.Get(); // complie error

    // EXPECT_EQ(str.c_str(), optStrRef->c_str()); // complie error

    Optional<String &> optStrRef2 = optStrRef; // move
    EXPECT_STREQ(optStrRef2.Get().c_str(), str.c_str());
    EXPECT_TRUE(optStrRef.IsNone());

    // optStrRef = NULL; // complie error
}

TEST(OptionalTest, OptionalConstStringPtr)
{
    String str = "test";
    Optional<const String *> optConstStrPtr = &str; // mut ptr

    // String &strRef = optConstStrPtr; // complie error

    // const String &constStrRef = optConstStrPtr; // complie error

    // String strDup = optConstStrPtr;// complie error

    // PCSTR pcstr = optConstStrPtr.Get(); // complie error

    // String *strPtr = optConstStrPtr; // complie error

    const String *constStrPtr = optConstStrPtr;
    EXPECT_EQ(&str, constStrPtr);

    EXPECT_EQ(str.c_str(), optConstStrPtr->c_str());

    // optConstStrPtr->clear(); // complie error

    Optional<const String *> optConstStrPtr2 = optConstStrPtr; // move
    EXPECT_STREQ(optConstStrPtr2.Get()->c_str(), str.c_str());
    EXPECT_TRUE(optConstStrPtr.IsNone());

    optConstStrPtr = NULL;
    // EXPECT_THROW(const String &constStrRef = optConstStrPtr, NullException); // complie error
    // EXPECT_THROW(strDup = optConstStrPtr, NullException);                    // complie error
    // EXPECT_NO_THROW(pcstr = optConstStrPtr.Get());                          // complie error
    // EXPECT_EQ(pcstr, (PCSTR)NULL);                                          // complie error
}

TEST(OptionalTest, OptionalStringPtr)
{
    String str = "test";
    Optional<String *> optStrPtr = &str; // mut ptr

    // String &strRef = optStrPtr; // complie error

    // const String &constStrRef = optStrPtr; // complie error

    // String strDup = optStrPtr;// complie error

    // PCSTR pcstr = optStrPtr.Get(); // complie error

    const String *strPtr = optStrPtr;
    EXPECT_EQ(&str, strPtr);

    const String *constStrPtr = optStrPtr;
    EXPECT_EQ(&str, constStrPtr);

    EXPECT_EQ(str.c_str(), optStrPtr->c_str());

    optStrPtr->clear();

    Optional<String *> optStrPtr2 = optStrPtr; // move
    EXPECT_STREQ(optStrPtr2.Get()->c_str(), str.c_str());
    EXPECT_TRUE(optStrPtr.IsNone());

    optStrPtr = NULL;
    // EXPECT_THROW(const String &constStrRef = optStrPtr, NullException); // complie error
    // EXPECT_THROW(strDup = optStrPtr, NullException);                    // complie error
    // EXPECT_NO_THROW(pcstr = optStrPtr.Get());                          // complie error
    // EXPECT_EQ(pcstr, (PCSTR)NULL);                                     // complie error
}

TEST(OptionalTest, OptionalPCSTR)
{
    String str = "test";

    Optional<PCSTR> optPCSTR = str.c_str();
    // String &strRef = optPCSTR; // complie error

    const String &constStrRef = (PCSTR)optPCSTR;
    EXPECT_NE(str.c_str(), constStrRef.c_str());
    EXPECT_STREQ(str.c_str(), constStrRef.c_str());

    String strDup = (PCSTR)optPCSTR;
    EXPECT_STREQ(str.c_str(), strDup.c_str());

    PCSTR strPtr = optPCSTR;
    EXPECT_EQ(str.c_str(), strPtr);

    // EXPECT_EQ(str.c_str(), optPCSTR->c_str()); // complie error

    Optional<PCSTR> optPCSTR2 = optPCSTR; // move
    EXPECT_STREQ(optPCSTR2.Get(), str.c_str());
    EXPECT_TRUE(optPCSTR.IsNone());

    optPCSTR = NULL;
#if defined(_MSVC)
    EXPECT_NO_THROW({
        __try
        {
            const String &constStrRef = (PCSTR)optPCSTR;
            strDup = optPCSTR;
        }
        __finally
        {
        }
    });
#endif
    EXPECT_NO_THROW(strPtr = optPCSTR);
    EXPECT_EQ(strPtr, (PCSTR)NULL);
}

TEST(OptionalTest, OptionalPSTR)
{
    String str = "test";

    Optional<PSTR> optPSTR = &str[0];
    // Optional<PSTR> optPSTR = str.c_str(); // complie error
    // String &strRef = optPSTR; // complie error

    const String &constStrRef = (PSTR)optPSTR;
    EXPECT_NE(str.c_str(), constStrRef.c_str());
    EXPECT_STREQ(str.c_str(), constStrRef.c_str());

    String strDup = (PSTR)optPSTR;
    EXPECT_STREQ(str.c_str(), strDup.c_str());

    PCSTR pstr = optPSTR;
    EXPECT_EQ(str.c_str(), pstr);

    PCSTR pcstr = optPSTR;
    EXPECT_EQ(str.c_str(), pcstr);

    // EXPECT_EQ(str.c_str(), optPSTR->c_str()); // complie error

    Optional<PSTR> optPSTR2 = optPSTR; // move
    EXPECT_STREQ(optPSTR2.Get(), str.c_str());
    EXPECT_TRUE(optPSTR.IsNone());

    optPSTR = NULL;
#if defined(_MSVC)
    EXPECT_NO_THROW({
        __try
        {
            const String &constStrRef = (PSTR)optPSTR;
            strDup = optPSTR;
        }
        __finally
        {
        }
    });
#endif
    EXPECT_NO_THROW(pstr = optPSTR);
    EXPECT_EQ(pstr, (PSTR)NULL);
    EXPECT_NO_THROW(pcstr = optPSTR);
    EXPECT_EQ(pcstr, (PCSTR)NULL);
}

TEST(OptionalTest, OptionalConstPrimitiveType)
{
    int num = 1;

    Optional<const int> optConstInt = num;

    // int &numRef = optConstInt; // complie error

    const int &constNumRef = optConstInt;
    EXPECT_NE(&num, &constNumRef);
    EXPECT_EQ(num, constNumRef);

    int numDup = optConstInt;
    EXPECT_EQ(num, numDup);

    // int numGet = optConstInt.Get(); // complie error

    Optional<const int> optConstInt2 = optConstInt; // move
    EXPECT_EQ(optConstInt2.Get(), num);
    EXPECT_TRUE(optConstInt.IsNone());
}

TEST(OptionalTest, OptionalPrimitiveType)
{
    int num = 1;

    Optional<int> optInt = num;

    int &numRef = optInt;
    EXPECT_NE(&num, &numRef);
    numRef = 2;
    EXPECT_EQ(optInt, numRef);
    numRef = num;
    EXPECT_EQ(optInt, num);

    const int &constNumRef = optInt;
    EXPECT_NE(&num, &constNumRef);
    EXPECT_EQ(num, constNumRef);

    int numDup = optInt;
    EXPECT_EQ(num, numDup);

    // int numGet = optInt.Get(); // complie error

    Optional<int> optInt2 = optInt; // move
    EXPECT_EQ(optInt2.Get(), num);
    EXPECT_TRUE(optInt.IsNone());
}

TEST(OptionalTest, OptionalConstPrimitiveTypeRef)
{
    int num = 1;

    Optional<const int &> optConstIntRef = num; // const ref

    // int &numRef = optConstIntRef; // complie error

    const int &constNumRef = optConstIntRef;
    EXPECT_EQ(&num, &constNumRef);

    int numDup = optConstIntRef;
    EXPECT_EQ(num, numDup);

    // int numGet = optConstIntRef.Get(); // complie error

    Optional<const int &> optConstIntRef2 = optConstIntRef; // move
    EXPECT_EQ(&optConstIntRef2.Get(), &num);
    EXPECT_TRUE(optConstIntRef.IsNone());
}

TEST(OptionalTest, OptionalPrimitiveTypeRef)
{
    int num = 1;

    Optional<int &> optIntRef = num; // ref

    int &numRef = optIntRef;
    EXPECT_EQ(&num, &numRef);
    numRef = 2;
    EXPECT_EQ(optIntRef, numRef);
    numRef = num;

    const int &constNumRef = optIntRef;
    EXPECT_EQ(&num, &constNumRef);
    EXPECT_EQ(num, constNumRef);

    int numDup = optIntRef;
    EXPECT_EQ(num, numDup);

    // int numGet = optIntRef.Get(); // complie error

    Optional<int &> optIntRef2 = optIntRef; // move
    EXPECT_EQ(&optIntRef2.Get(), &num);
    EXPECT_TRUE(optIntRef.IsNone());
}

TEST(OptionalTest, OptionalConstPrimitiveTypePtr)
{
    int num = 1;

    Optional<const int *> optConstIntPtr = &num; // const ptr

    // int *numPtr = optConstIntPtr; // complie error

    const int *constNumPtr = optConstIntPtr;
    EXPECT_EQ(&num, constNumPtr);

    // int &numRef = optConstIntPtr; // complie error

    // const int &constNumRef = optConstIntPtr; // complie error

    // int numDup = optConstIntPtr; // complie error

    // int numGet = optConstIntPtr.Get(); // complie error

    Optional<const int *> optConstIntPtr2 = optConstIntPtr; // move
    EXPECT_EQ(optConstIntPtr2.Get(), &num);
    EXPECT_TRUE(optConstIntPtr.IsNone());
}

TEST(OptionalTest, OptionalPrimitiveTypePtr)
{
    int num = 1;

    Optional<int *> optIntPtr = &num; // ptr

    int *numPtr = optIntPtr;
    EXPECT_EQ(&num, numPtr);

    const int *constNumPtr = optIntPtr;
    EXPECT_EQ(&num, constNumPtr);

    // int &numRef = optIntPtr; // complie error

    // const int &constNumRef = optIntPtr; // complie error

    // int numDup = optIntPtr; // complie error

    // int numGet = optIntPtr.Get(); // complie error

    Optional<int *> optIntPtr2 = optIntPtr; // move
    EXPECT_EQ(optIntPtr2.Get(), &num);
    EXPECT_TRUE(optIntPtr.IsNone());
}

TEST(OptionalTest, OptionalStringClone)
{
    Optional<String> optStr = "Test";
    Optional<String> optStr2;
    EXPECT_STREQ(optStr.Get(), "Test");
    EXPECT_TRUE(optStr2.IsNone());

    optStr2 = optStr;
    EXPECT_TRUE(optStr.IsNone());
    EXPECT_STREQ(optStr2.Get(), "Test");

    Optional<String> optStr3 = optStr2.Clone();
    EXPECT_STREQ(optStr2.Get(), "Test");
    EXPECT_STREQ(optStr3.Get(), "Test");

    optStr = optStr2.Clone();
    EXPECT_STREQ(optStr2.Get(), "Test");
    EXPECT_STREQ(optStr.Get(), "Test");

    optStr3 = optStr2;
    EXPECT_TRUE(optStr2.IsNone());
    EXPECT_STREQ(optStr3.Get(), "Test");
}

TEST(OptionalTest, OptionalStringRefClone)
{
    String val = "Test";
    Optional<String &> optStr = val;
    Optional<String &> optStr2;
    EXPECT_EQ(((String &)optStr).c_str(), val.c_str());
    EXPECT_TRUE(optStr2.IsNone());

    optStr2 = optStr;
    EXPECT_TRUE(optStr.IsNone());
    EXPECT_EQ(((String &)optStr2).c_str(), val.c_str());

    Optional<String &> optStr3 = optStr2.Clone();
    EXPECT_EQ(((String &)optStr2).c_str(), val.c_str());
    EXPECT_EQ(((String &)optStr3).c_str(), val.c_str());

    val = "TestNew";
    EXPECT_STREQ(((String &)optStr2).c_str(), "TestNew");
    EXPECT_STREQ(((String &)optStr3).c_str(), "TestNew");

    optStr = optStr2.Clone();
    EXPECT_EQ(((String &)optStr2).c_str(), val.c_str());
    EXPECT_EQ(((String &)optStr).c_str(), val.c_str());

    optStr3 = optStr2;
    EXPECT_TRUE(optStr2.IsNone());
    EXPECT_EQ(((String &)optStr3).c_str(), val.c_str());
}

TEST(OptionalTest, OptionalConstStringRefClone)
{
    String val = "Test";
    Optional<const String &> optStr = val;
    Optional<const String &> optStr2;
    EXPECT_EQ(((const String &)optStr).c_str(), val.c_str());
    EXPECT_TRUE(optStr2.IsNone());

    optStr2 = optStr;
    EXPECT_TRUE(optStr.IsNone());
    EXPECT_EQ(((const String &)optStr2).c_str(), val.c_str());

    Optional<const String &> optStr3 = optStr2.Clone();
    EXPECT_EQ(((const String &)optStr2).c_str(), val.c_str());
    EXPECT_EQ(((const String &)optStr3).c_str(), val.c_str());

    optStr = optStr2.Clone();
    EXPECT_EQ(((const String &)optStr2).c_str(), val.c_str());
    EXPECT_EQ(((const String &)optStr).c_str(), val.c_str());

    optStr3 = optStr2;
    EXPECT_TRUE(optStr2.IsNone());
    EXPECT_EQ(((const String &)optStr3).c_str(), val.c_str());
}

TEST(OptionalTest, OptionalPrimitiveTypeClone)
{
    Optional<int> optNum = 1;
    Optional<int> optNum2 = None();
    EXPECT_EQ(optNum.Get(), 1);
    EXPECT_TRUE(optNum2.IsNone());

    optNum2 = optNum;
    EXPECT_TRUE(optNum.IsNone());
    EXPECT_EQ(optNum2.Get(), 1);

    Optional<int> optNum3 = optNum2.Clone();
    EXPECT_EQ(optNum2.Get(), 1);
    EXPECT_EQ(optNum3.Get(), 1);

    optNum = optNum2.Clone();
    EXPECT_EQ(optNum2.Get(), 1);
    EXPECT_EQ(optNum.Get(), 1);

    optNum3 = optNum2;
    EXPECT_TRUE(optNum2.IsNone());
    EXPECT_EQ(optNum3.Get(), 1);
}

TEST(OptionalTest, OptionalPrimitiveRefTypeClone)
{
    int val = 1;
    Optional<int &> opt = val;
    Optional<int &> opt2;
    EXPECT_EQ(&((int &)opt), &val);
    EXPECT_TRUE(opt2.IsNone());

    opt2 = opt;
    EXPECT_TRUE(opt.IsNone());
    EXPECT_EQ(&((int &)opt2), &val);

    Optional<int &> opt3 = opt2.Clone();
    EXPECT_EQ(&((int &)opt2), &val);
    EXPECT_EQ(&((int &)opt3), &val);

    val = 2;
    EXPECT_EQ(((int &)opt2), 2);
    EXPECT_EQ(((int &)opt3), 2);

    opt = opt2.Clone();
    EXPECT_EQ(&((int &)opt2), &val);
    EXPECT_EQ(&((int &)opt), &val);

    opt3 = opt2;
    EXPECT_TRUE(opt2.IsNone());
    EXPECT_EQ(&((int &)opt3), &val);
}
