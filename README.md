# Win32Ex

Win32 API Experimental(or Extension) features.

[![CMake](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml) [![MSYS2](https://github.com/ntoskrnl7/win32-ex/actions/workflows/msys2.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/msys2.yml) ![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/ntoskrnl7/win32-ex)

This library is designed for use in legacy development environments. So it should support Visual Studio 2008 SP1 (probably C++03).

---

## Requirements

- Windows 8 or later
- Visual Studio 2008 SP1 or later
- MSYS2
  - MSYS
  - MinGW 32-bit
  - MinGW 64-bit
  - MinGW Clang 32-bit
  - MinGW Clang 64-bit
  - MinGW UCRT 64-bit

---

## Test Environments

- Windows 10
- MSYS2
  - MSYS
  - MinGW 32-bit
  - MinGW 64-bit
  - MinGW Clang 32-bit
  - MinGW Clang 64-bit
  - MinGW UCRT 64-bit
- Visual Studio
  - 2008 SP1
  - 2010, 2012, 2017, 2019, 2022

---

## Contents

- [Win32Ex](#win32ex)
  - [Requirements](#requirements)
  - [Test Environments](#test-environments)
  - [Contents](#contents)
  - [Features](#features)
    - [System Services](#system-services)
      - [Processes and Threads](#processes-and-threads)
      - [Remote Desktop Services](#remote-desktop-services)
      - [Services](#services)
      - [Windows System Information](#windows-system-information)
        - [Handles and Objects](#handles-and-objects)
    - [Security and Identity](#security-and-identity)
      - [Privileges](#privileges)
      - [Access Tokens](#access-tokens)
      - [Security Descriptors](#security-descriptors)
      - [Security Identifiers](#security-identifiers)
    - [Etc](#etc)
      - [Optional](#optional)
      - [Result](#result)
      - [Win32 API Template](#win32-api-template)
  - [Test](#test)
    - [CMake](#cmake)
  - [Usage](#usage)
    - [CMakeLists.txt](#cmakeliststxt)
    - [test.cpp](#testcpp)

---

## Features

- **Link :** <https://docs.microsoft.com/windows/win32/desktop-app-technologies>
- **Headers :** Win32Ex.h

### System Services

- **Link :** <https://docs.microsoft.com/windows/win32/system-services>
- **Headers :** Win32Ex/System.h
- [More](docs/system-services.md)

#### Processes and Threads

- **Link :** <https://docs.microsoft.com/windows/win32/procthread/processes-and-threads>
- **Headers :** Win32Ex/System/Process.h, Win32Ex/System/Process.hpp
- [More](docs/procthread.md)

#### Remote Desktop Services

- **Link :** <https://docs.microsoft.com/windows/win32/termserv/terminal-services-portal>
- **Headers :** Win32Ex/System/Session.hpp
- [More](docs/termserv.md)

#### Services

- **Link :** <https://docs.microsoft.com/windows/win32/services/services>
- **Headers :** Win32Ex/System/Service.hpp
- [More](docs/services.md)

#### Windows System Information

- **Link :** <https://docs.microsoft.com/windows/win32/sysinfo/windows-system-information>
- [More](docs/sysinfo.md)

##### Handles and Objects

- **Link :** <https://docs.microsoft.com/windows/win32/sysinfo/handles-and-objects>
- **Headers :** Win32Ex/System/Object.h
- [More](docs/sysinfo/handles-and-objects.md)

### Security and Identity

- **Link :** <https://docs.microsoft.com/windows/win32/secauthz/authorization-portal>
- **Headers :** Win32Ex/Security.h
- [More](docs/secauthz.md)

#### Privileges

- **Link :** <https://docs.microsoft.com/windows/win32/secauthz/privileges>
- **Headers :** Win32Ex/Security/Privilege.h, Win32Ex/Security/Privilege.hpp
- [More](docs/secauthz/privileges.md)

#### Access Tokens

- **Link :** <https://docs.microsoft.com/windows/win32/secauthz/access-tokens>
- **Headers :** Win32Ex/Security/Token.h, Win32Ex/Security/Token.hpp
- [More](docs/secauthz/access-tokens.md)

#### Security Descriptors

- **Link :** <https://docs.microsoft.com/windows/win32/secauthz/security-descriptors>
- **Headers :** Win32Ex/Security/Descriptor.h
- [More](docs/secauthz/security-descriptors.md)

#### Security Identifiers

- **Link :** <https://docs.microsoft.com/windows/win32/secauthz/security-identifiers>
- **Headers :** Security\Sid.h
- [More](docs/secauthz/security-identifiers.md)

### Etc

- [More](docs/etc.md)

#### Optional

- **Headers :** Win32Ex/Optional.hpp
- [More](docs/etc/optional.md)

#### Result

- **Headers :** Win32Ex/Result.hpp
- [More](docs/etc/result.md)

#### Win32 API Template

- [More](docs/etc/api-tmpl.md)

---

## Test

### CMake

- With MSYS2

  ```bash
  cd test
  mkdir build && cd build
  if [ "$MSYSTEM" = "MSYS" ]; then
      cmake ..
  else
      cmake -G "MinGW Makefiles" ..
  fi

  cmake --build .

  if [ "$?" -eq 0 ]; then
      export LC_ALL=C; unset LANGUAGE
      ctest . --verbose
  fi
  ```

- With Visual Studio

  ```batch
  cd test
  mkdir build
  cd build
  cmake ..
  cmake --build .  -- /p:CharacterSet=Unicode
  if errorlevel 0 ctest . --verbose -C Debug
  ```

## Usage

*We usually recommend using CMake. However, if you are using Visual Studio directly, add **{This repository}/include** to the '**[Additional Include Directories](https://docs.microsoft.com/cpp/build/reference/i-additional-include-directories#to-set-this-compiler-option-in-the-visual-studio-development-environment
)** property'.*

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# create project
project(MyProject)

# add executable
add_executable(tests tests.cpp)

# add dependencies
include(cmake/CPM.cmake)
CPMAddPackage("gh:ntoskrnl7/win32-ex@0.8.17")

# link dependencies
target_link_libraries(tests win32ex)
```

### test.cpp

```C++
#include <iostream>
#include <Win32Ex/System/Process.hpp>
// or #include <Win32Ex.h>
// or #include <Win32Ex/System.h>

int main()
{
    std::cout << Win32Ex::System::Process::All().size();
    return 0;
}
```
