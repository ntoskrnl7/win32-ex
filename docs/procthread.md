# Processes and threads

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/procthread/processes-and-threads>
- **Headers :** Win32Ex/System/Process.h, Win32Ex/System/Process.hpp

## Contents

- [Processes and threads](#processes-and-threads)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Functions](#functions)
      - [CreateUserAccountProcess](#createuseraccountprocess)
      - [CreateSystemAccountProcess](#createsystemaccountprocess)
      - [CreateUserAccountProcessT\<typename CharType\>](#createuseraccountprocessttypename-chartype)
      - [CreateSystemAccountProcessT\<typename CharType\>](#createsystemaccountprocessttypename-chartype)
    - [Classes](#classes)
      - [Process](#process)
      - [ProcessW](#processw)
      - [ProcessT\<class StringType = Win32Ex::StringT\>](#processtclass-stringtype--win32exstringt)
      - [RunnableProcess](#runnableprocess)
      - [RunnableProcessW](#runnableprocessw)
      - [RunnableProcessT\<class StringType = Win32Ex::StringT\>](#runnableprocesstclass-stringtype--win32exstringt)
      - [UserAccountProcess](#useraccountprocess)
      - [UserAccountProcessW](#useraccountprocessw)
      - [UserAccountProcessT](#useraccountprocesst)
      - [SystemAccountProcess](#systemaccountprocess)
      - [SystemAccountProcessW](#systemaccountprocessw)
      - [SystemAccountProcessT](#systemaccountprocesst)
      - [RunnableSessionProcessT\<ProcessAccountType Type, class StringType = Win32Ex::StringT\>](#runnablesessionprocesstprocessaccounttype-type-class-stringtype--win32exstringt)
      - [ElevatedProcess](#elevatedprocess)
      - [ElevatedProcessW](#elevatedprocessw)
      - [ElevatedProcessT\<class StringType = Win32Ex::StringT\>](#elevatedprocesstclass-stringtype--win32exstringt)
    - [Namespaces](#namespaces)
      - [ThisProcess](#thisprocess)

## Reference

### Functions

#### CreateUserAccountProcess
 
- It's better using [Wini32Ex::System::Session::NewProcess method](./termserv.md#session) than this funtion.
- Example
  - Creating a user account process in the current session.

    ```C
    #include <Win32Ex/System/Process.h>

    CreateUserAccountProcess(WTSGetActiveConsoleSessionId(), NULL, TEXT("CMD.exe /C QUERY SESSION"), /* ... */);    
    ```

#### CreateSystemAccountProcess

- It's better using [Wini32Ex::System::Session::NewProcess method](./termserv.md#session) than this funtion.
- Example
  - Creating a system account process in the current session.

    ```C
    #include <Win32Ex/System/Process.h>

    CreateSystemAccountProcess(WTSGetActiveConsoleSessionId(), NULL, TEXT("CMD.exe /C QUERY SESSION"), /* ... */);    
    ```

#### CreateUserAccountProcessT\<typename CharType\>

- **C++ only**
- It's better using [Wini32Ex::System::Session::NewProcess method](./termserv.md#session) than this funtion.
- Example
  - Creating a user account process at the current session.

    ```C++
    #include <Win32Ex/System/Process.h>

    CreateUserAccountProcessT<CHAR>(WTSGetActiveConsoleSessionId(), NULL, "CMD.exe /C QUERY SESSION", /* ... */);
    ```

#### CreateSystemAccountProcessT\<typename CharType\>

- **C++ only**
- It's better using [Wini32Ex::System::Session::NewProcess method](./termserv.md#session) than this funtion.
- Example
  - Creating a system account process at the current session.

    ```C++
    #include <Win32Ex/System/Process.h>

    CreateSystemAccountProcessT<CHAR>(WTSGetActiveConsoleSessionId(), NULL, "CMD.exe /C QUERY SESSION", /* ... */);
    ```

### Classes

#### Process

- ProcessT\<Win32Ex::String\>
- Example
  - Attach Process by process id, process handle.

      ```C++
      #include <Win32Ex/System/Process.hpp>

      DWORD pid = ...
      Win32Ex::System::Process process(pid);
      if (process.IsAttached())
      {
      process.ExecutablePath();
      }

      HANDLE hProcess = ..
      Win32Ex::System::Process process(Win32Ex::System::ProcessHandle::FromHANDLE(hProcess));
      if (process.IsAttached())
      {
      process.ExecutablePath();
      }
      ```

  - Enumerate all processes.

      ```C++
      #include <Win32Ex/System/Process.hpp>

      for (auto process : Win32Ex::System::Process::All())
      {
          if (process)
          {
              if (process->IsAttached())
                  std::cout << "[Attached] PID :" << process->Id() << "\t\tPATH : " << process->ExecutablePath() << '\n';
              else
                  std::cout << "PID :" << process->Id() << "\t\tPATH : " << process->ExecutablePath() << '\n';
          }
          }
      ```

#### ProcessW

- ProcessT\<Win32Ex::StringW\>

#### ProcessT\<class StringType = Win32Ex::StringT\>

- Implements WaitableObject.

#### RunnableProcess

- **Abstract**
- RunnableProcessT\<Win32Ex::String\>
- Example
  - Use the RunnableProcess class to run 'user account process'.

      ```C++
      #include <Win32Ex/System/Process.hpp>
      // We usually recommend using the Wini32Ex::System::Session::NewSession method.
      Win32Ex::System::UserAccountProcess process("CMD", "/C WHOAMI");
      Win32Ex::System::RunnableProcess &runnable = process;
      runnable.Run();
      ```

#### RunnableProcessW

- **Abstract**
- RunnableProcessT\<Win32Ex::StringW\>

#### RunnableProcessT\<class StringType = Win32Ex::StringT\>

- **Abstract**
- Implements ProcessT.

#### UserAccountProcess

- RunnableSessionProcessT\<UserAccount, Win32Ex::String\>
- It's better using [Wini32Ex::System::Session::NewProcess method](./termserv.md#session) than this class.
- Example
  - Creating a user account process at the current session.

      ```C++
      #include <Win32Ex/System/Process.hpp>

      Win32Ex::System::UserAccountProcess process("CMD.exe");
      process.Run("/C QUERY SESSION");

      auto waiter = process.RunAsync("/C QUERY SESSION");
      waiter.Wait();
      ```

  - Runs user account process at each sessions.

      ```C++
      #include <Win32Ex/System/Process.hpp>

      PWTS_SESSION_INFO sessionInfo = NULL;
      DWORD count = 0;
      if (WTSEnumerateSessions(WTS_CURRENT_SERVER, 0, 1, &sessionInfo, &count))
      {
      for (DWORD i = 0; i < count; ++i)
      {
          Win32Ex::System::UserAccountProcess process(sessionInfo[i].SessionId, "CMD", "/C QUERY SESSION");
          process.Run();
      }
      }
      ```

#### UserAccountProcessW

- RunnableSessionProcessT\<UserAccount, Win32Ex::StringW\>
- It's better using [Wini32Ex::System::Session::NewProcess method](./termserv.md#session) than this class.

#### UserAccountProcessT

- Implements RunnableProcessT.
- It's better using [Wini32Ex::System::Session::NewProcess method](./termserv.md#session) than this class.

#### SystemAccountProcess

- RunnableSessionProcessT\<SystemAccount, Win32Ex::String\>
- It's better using [Wini32Ex::System::Session::NewProcess method](./termserv.md#session) than this class.

#### SystemAccountProcessW

- RunnableSessionProcessT\<SystemAccount, Win32Ex::StringW\>
- It's better using [Wini32Ex::System::Session::NewProcess method](./termserv.md#session) than this class.

#### SystemAccountProcessT

- Implements RunnableProcessT.

#### RunnableSessionProcessT\<ProcessAccountType Type, class StringType = Win32Ex::StringT\>

- It's better using [Wini32Ex::System::Session::NewProcess method](./termserv.md#session) than this class.

#### ElevatedProcess

- ElevatedProcessT\<Win32Ex::String\>
- Example
  - Creating a with elevated permissions UAC.

      ```C++
      #include <Win32Ex/System/Process.hpp>

      Win32Ex::System::ElevatedProcess process("notepad.exe");
      process.Run();
      ```

#### ElevatedProcessW

- ElevatedProcessT\<Win32Ex::StringW\>

#### ElevatedProcessT\<class StringType = Win32Ex::StringT\>

- Implements RunnableProcessT.

### Namespaces

#### ThisProcess

- Example
  - Enumerate parent processes.

    ```C++
    #include <Win32Ex/System/Process.hpp>

    Win32Ex::System::Process parent = Win32Ex::ThisProcess::Parent();
    while (parent.IsValid())
    {
        std::cout << parent.ExecutablePath() << '\n';
        parent = parent.Parent();
    }
    ```
