# Remote Desktop Services

- **Link :** <https://docs.microsoft.com/windows/win32/termserv/terminal-services-portal>
- **Headers :** Win32Ex/System/Session.hpp

## Contents

- [Remote Desktop Services](#remote-desktop-services)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Classes](#classes)
      - [Session](#session)
      - [SessionW](#sessionw)
      - [SessionT\<class StringType = Win32Ex::StringT\>](#sessiontclass-stringtype--win32exstringt)
    - [Namespaces](#namespaces)
      - [ThisSession](#thissession)

## Reference

### Classes

#### Session

- SessionT\<Win32Ex::String\>
- Example
  - Creating a system account process and user account process in each sessions.

      ```C++
      for (auto session : Win32Ex::System::Session::All())
      {
          auto process = session->NewProcess(Win32Ex::System::UserAccount, "notepad");
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
      ```

#### SessionW

- SessionT\<Win32Ex::StringW\>

#### SessionT\<class StringType = Win32Ex::StringT\>

### Namespaces

#### ThisSession

- Example
  - Get the name of this session.

      ```C++
      Win32Ex::ThisSession::Name();
      Win32Ex::ThisSession::NameW();
      Win32Ex::ThisSession::NameT();
      Win32Ex::ThisSession::NameT<Win32Ex::String>();
      ```

  - Creating a system account process and user account process in this session.

      ```C++
      auto process = Win32Ex::ThisSession::NewProcess(Win32Ex::System::UserAccount, "notepad");
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
      ```
