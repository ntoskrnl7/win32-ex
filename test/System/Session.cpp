#include <Win32Ex/System/Session.hpp>
#include <gtest/gtest.h>

TEST(SessionTest, ThisSession)
{
    EXPECT_EQ(Win32Ex::ThisSession::Id(), WTSGetActiveConsoleSessionId());
    EXPECT_STREQ(Win32Ex::ThisSession::Name().c_str(),
                 Win32Ex::System::Session(Win32Ex::ThisSession::Id()).Name().c_str());
    EXPECT_STREQ(Win32Ex::ThisSession::UserName().c_str(),
                 Win32Ex::System::Session(Win32Ex::ThisSession::Id()).UserName().c_str());
    EXPECT_EQ(Win32Ex::ThisSession::State(), WTSActive);
}

TEST(SessionTest, ThisSessionNewProcessT)
{
#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
    auto process = Win32Ex::ThisSession::NewProcessT(Win32Ex::System::UserAccount, TEXT("notepad"));
#else
    Win32Ex::Result<Win32Ex::System::SessionT<>::RunnableProcessPtr> process =
        Win32Ex::ThisSession::NewProcessT<Win32Ex::StringT>(Win32Ex::System::UserAccount, TEXT("notepad"));
#endif
    if (process.IsOk())
    {
        process->RunAsync().Wait(500);
        process->Exit();
    }
    process = Win32Ex::ThisSession::NewProcessT<Win32Ex::StringT>(Win32Ex::System::SystemAccount, TEXT("notepad"));
    if (process.IsOk())
    {
        process->RunAsync().Wait(500);
        process->Exit();
    }
}

TEST(SessionTest, ThisSessionNewProcess)
{
#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
    auto process = Win32Ex::ThisSession::NewProcess(Win32Ex::System::UserAccount, "notepad");
#else
    Win32Ex::Result<Win32Ex::System::Session::RunnableProcessPtr> process =
        Win32Ex::ThisSession::NewProcess(Win32Ex::System::UserAccount, "notepad");
#endif
    if (process.IsOk())
    {
        process->RunAsync().Wait(500);
        process->Exit();
    }
    process = Win32Ex::ThisSession::NewProcess(Win32Ex::System::SystemAccount, "notepad");
    if (process.IsOk())
    {
        process->RunAsync().Wait(500);
        process->Exit();
    }
}

TEST(SessionTest, ThisSessionNewProcessW)
{
#if defined(WIN32EX_USE_TEMPLATE_FUNCTION_DEFAULT_ARGUMENT_STRING_T)
    auto process = Win32Ex::ThisSession::NewProcessW(Win32Ex::System::UserAccount, L"notepad");
#else
    Win32Ex::Result<Win32Ex::System::SessionW::RunnableProcessPtr> process =
        Win32Ex::ThisSession::NewProcessW(Win32Ex::System::UserAccount, L"notepad");
#endif
    if (process.IsOk())
    {
        process->RunAsync().Wait(500);
        process->Exit();
    }
    process = Win32Ex::ThisSession::NewProcessW(Win32Ex::System::SystemAccount, L"notepad");
    if (process.IsOk())
    {
        process->RunAsync().Wait(500);
        process->Exit();
    }
}

TEST(SessionTest, SessionNewProcess)
{
#if defined(__cpp_range_based_for)
    for (auto session : Win32Ex::System::Session::All())
    {
        auto process = session->NewProcess(Win32Ex::System::UserAccount, "notepad");
#else
    // clang-format off
    for each (const Win32Ex::SharedPtr<Win32Ex::System::Session> &session in Win32Ex::System::Session::All())
    {
        Win32Ex::Result<Win32Ex::System::Session::RunnableProcessPtr> process =
            session->NewProcess(Win32Ex::System::UserAccount, "notepad");
            // clang-format on
#endif
        if (process.IsOk())
        {
            process->RunAsync().Wait(500);
            process->Exit();
        }
        process = session->NewProcess(Win32Ex::System::SystemAccount, "notepad");
        if (process.IsOk())
        {
            process->RunAsync().Wait(500);
            process->Exit();
        }
    }
}

TEST(SessionTest, SessionWNewProcess)
{
#if defined(__cpp_range_based_for)
    for (auto session : Win32Ex::System::SessionW::All())
    {
        auto process = session->NewProcess(Win32Ex::System::UserAccount, L"notepad");
#else
    // clang-format off
    for each (const Win32Ex::SharedPtr<Win32Ex::System::SessionW> &session in Win32Ex::System::SessionW::All())
    {
        Win32Ex::Result<Win32Ex::System::SessionW::RunnableProcessPtr> process =
            session->NewProcess(Win32Ex::System::UserAccount, L"notepad");
            // clang-format on
#endif
        if (process.IsOk())
        {
            process->RunAsync().Wait(500);
            process->Exit();
        }
        process = session->NewProcess(Win32Ex::System::SystemAccount, L"notepad");
        if (process.IsOk())
        {
            process->RunAsync().Wait(500);
            process->Exit();
        }
    }
}

TEST(SessionTest, SessionTNewProcess)
{
#if defined(__cpp_range_based_for)
    for (auto session : Win32Ex::System::SessionT<>::All())
    {
        auto process = session->NewProcess(Win32Ex::System::UserAccount, TEXT("notepad"));
#else
    // clang-format off
    for each (const Win32Ex::SharedPtr<Win32Ex::System::SessionT<>> &session in Win32Ex::System::SessionT<>::All())
    {
        Win32Ex::Result<Win32Ex::System::SessionT<>::RunnableProcessPtr> process =
            session->NewProcess(Win32Ex::System::UserAccount, TEXT("notepad"));
            // clang-format on
#endif
        if (process.IsOk())
        {
            process->RunAsync().Wait(500);
            process->Exit();
        }
        process = session->NewProcess(Win32Ex::System::SystemAccount, TEXT("notepad"));
        if (process.IsOk())
        {
            process->RunAsync().Wait(500);
            process->Exit();
        }
    }
}
