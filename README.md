# Win32Ex

Win32 API Experimental(or Extension) features.

[![CMake](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/cmake.yml)
[![MSYS2](https://github.com/ntoskrnl7/win32-ex/actions/workflows/msys2.yml/badge.svg)](https://github.com/ntoskrnl7/win32-ex/actions/workflows/msys2.yml)
![GitHub](https://img.shields.io/github/license/ntoskrnl7/win32-ex)
![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/ntoskrnl7/win32-ex)
![Windows 8+](https://img.shields.io/badge/Windows-8+-blue?logo=windows&logoColor=white)
![Visual Studio 2008 SP1+](https://img.shields.io/badge/Visual%20Studio-2008%20SP1+-682270?logo=visualstudio&logoColor=682270)
![MSYS2 MSYS](https://img.shields.io/badge/MSYS2%20-MSYS-magenta.svg?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAMAAAAoLQ9TAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAA1VBMVEWJTISNUoizi6+xia6KTYWMUYeoeqSicZ6RWIz7+fv///+9m7uJTYTezd3ax9ijdJ/l1+SqfqbUvtPVv9OKToXezNzQuM6TW478+vz38/ebZ5acaJf//v/NtMvBob6+nLuwh63Qt87x6fDz7fLRus+OU4n18PTm2uW/nbzs4+zj1OK0jbH+/f7+/v759/mQV4zm2eXz7fPz7PKMUIfp3ujNssq7mLjBoL7Hq8X9+/2XYpPx6vGLT4b6+PrTvNGPVouQV4uuhKuLUIahcJ2VXpGsgaiRWY0Ag/KpAAAAAWJLR0QKaND0VgAAAAd0SU1FB+YFGwwlNxplgV0AAACaSURBVBjTjc9HFoIADATQwQqDvYsdsYEFRVHB3u5/JGkHIJu8+Yt5CRBrhEQy5a10JhuBKFEGcnkWIiiSJaBMViKoklINdbIR5marTSqdLtkLoc8BORypY2pBnkxnCqlJ8wX1AGQawpKr9cakJPpHbHcW9uTB77Y9sHkETjw7MEhXwMU1BcC53gBLp3rH4/nyi94f//bvL8ajfz3MDQMfR5tdAAAAJXRFWHRkYXRlOmNyZWF0ZQAyMDIyLTA1LTI3VDEyOjM3OjU1KzAwOjAwqTZC/AAAACV0RVh0ZGF0ZTptb2RpZnkAMjAyMi0wNS0yN1QxMjozNzo1NSswMDowMNhr+kAAAAAASUVORK5CYII=)
![MSYS2 MinGW 32 bit](https://img.shields.io/badge/MSYS2-MinGW%2032-white.svg?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAMAAAAoLQ9TAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAzFBMVEV0h4d5i4ulsbGjsLB1iIh4ioqYpqaRoKB9j4/6+/v///+yvLx1h4fY3t7T2dmToqLg5OSbqKjN1NR2iIjY3d3Iz89/kZH7/Pz29/eJmZmKmpr+///Fzc22wMCyvb2ir6/Hz8/u8PDx8/PJ0NB5jIzz9fXi5ub7+/uzvr7p7Oze4uKns7N4i4v9/v7+/v74+fnw8vJ3iorl6enEzMyvurq1v7++x8f8/f2Flpbv8fF2iYn6+vrL0tJ8jo6gra13iYmQn5+Ck5Odqqp+j48jrK5jAAAAAWJLR0QKaND0VgAAAAd0SU1FB+YFGwwlNxplgV0AAACaSURBVBjTjc9HFoIADATQscNgxYodUFTAgqKCivX+d5J2ALLJm7+YlwCZJpcvFMNVKldSEERKQLXGegoNsgm0SDkFmRTb6JDdJPf6A1IZjshxAhNOydlc1ajHebE0FFJfrTc0Y5Bo2Vvu9oJBUQizrR0cHMlT1O2G4PIMXHj1YJG+jZtv2IB3fwCOSTVA8HxFRe9PdPv3l+HRP06iDTTK47P9AAAAJXRFWHRkYXRlOmNyZWF0ZQAyMDIyLTA1LTI3VDEyOjM3OjU0KzAwOjAwD0FJSAAAACV0RVh0ZGF0ZTptb2RpZnkAMjAyMi0wNS0yN1QxMjozNzo1NCswMDowMH4c8fQAAAAASUVORK5CYII=)
![MSYS2 MinGW 64 bit](https://img.shields.io/badge/MSYS2-MinGW%2064-blue.svg?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAMAAAAoLQ9TAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAA2FBMVEUnZZcvaptzm7xxmropZpgtaZpfjbJUha01b573+vv///+HqcUoZpfD1OK7z95Xh67P3ehjkLSxx9myyNoqZ5jC1OKpwtY5cqD5+/zx9fhHfKZJfaf+/v+kvtONrsiIqsVumLmpwdXl7PLp7/Srw9cva5vs8vbS3+n4+vyJq8bd5+/L2uZ2nb0uapr8/f79/v71+PrR3unq8PXo7/QsaZnX4+yjvdOCpsONrciZt877/P1BeKTm7fMqZ5n3+fuvxtgzbZ00bp1rlbgraJlShKw9daJnkrY2cJ5w8qACAAAAAWJLR0QKaND0VgAAAAd0SU1FB+YFGwwlNxplgV0AAACaSURBVBjTjc9HFoIADATQscNg72JHrICKoqjY6/1vJO0AZJM3fzEvASJNLJ5IuiuVzoQgiJSAbI75EApkESiR5RAqpFhFjawHudFskXK7Q3YD6LFPDobKiKqfx5OpTKqz+YKaDxJ1Y8nVWjApCm42NlsLO3Lvddsu2DwAR54c6OTZwOVsGoBzvQGWRuWOx/PlFb0/3u3fX4RH/4SWDdOSsTi4AAAAJXRFWHRkYXRlOmNyZWF0ZQAyMDIyLTA1LTI3VDEyOjM3OjU0KzAwOjAwD0FJSAAAACV0RVh0ZGF0ZTptb2RpZnkAMjAyMi0wNS0yN1QxMjozNzo1NCswMDowMH4c8fQAAAAASUVORK5CYII=)
![MSYS2 Clang 32 bit](https://img.shields.io/badge/MSYS2-Clang%2032-orange.svg?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAMAAAAoLQ9TAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAz1BMVEW+ZD7AaUXVm4LUmYC/ZUDAaEPPjHDMhGbCbkv9+vj////bqZS+ZT/t1MnrzsLNh2nw3NTQj3Tox7noyLq/ZkDt08nlwrPDcU79+/r79fLIe1vIfFz//v7kvq7drprTl37lwbL37Oj47+vmw7TBakb58u7x39f9+vncq5b15+Hv2tHWnYTAaUT+/f3+/v38+PbCbkrx3tb58Ozz4tvjvazappDdrZngtqT+/PvGd1X37ei/ZkH8+ffnxbfCbUnSlXu/Z0LLg2XFdFLRknfDb0xS4XEfAAAAAWJLR0QKaND0VgAAAAd0SU1FB+YFGwwlNm1iscsAAACaSURBVBjTjY9XEoIAFANjQSHYu1gBK2BBRQUV6/3PJO0AvJ/M7kcmD0h1mWwuH4RQKCZClCgDpTIriaiSNaBONhLRJKUW2mQn5m6vTyqDITmKxZgyOZmqGvWIZ/OFQurL1ZpGJGSa1obb3d6mJAZsHTQBR/IUdjuBcHgGLry6MEnPws2zLcC9P4KlBlUf/vMVFr0/4fbvL8Wjf1ClDS2qO1moAAAAJXRFWHRkYXRlOmNyZWF0ZQAyMDIyLTA1LTI3VDEyOjM3OjU0KzAwOjAwD0FJSAAAACV0RVh0ZGF0ZTptb2RpZnkAMjAyMi0wNS0yN1QxMjozNzo1NCswMDowMH4c8fQAAAAASUVORK5CYII=)
![MSYS2 Clang 64 bit](https://img.shields.io/badge/MSYS2-Clang%2064-orange.svg?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAMAAAAoLQ9TAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAz1BMVEW+ZD7AaUXVm4LUmYC/ZUDAaEPPjHDMhGbCbkv9+vj////bqZS+ZT/t1MnrzsLNh2nw3NTQj3Tox7noyLq/ZkDt08nlwrPDcU79+/r79fLIe1vIfFz//v7kvq7drprTl37lwbL37Oj47+vmw7TBakb58u7x39f9+vncq5b15+Hv2tHWnYTAaUT+/f3+/v38+PbCbkrx3tb58Ozz4tvjvazappDdrZngtqT+/PvGd1X37ei/ZkH8+ffnxbfCbUnSlXu/Z0LLg2XFdFLRknfDb0xS4XEfAAAAAWJLR0QKaND0VgAAAAd0SU1FB+YFGwwlNm1iscsAAACaSURBVBjTjY9XEoIAFANjQSHYu1gBK2BBRQUV6/3PJO0AvJ/M7kcmD0h1mWwuH4RQKCZClCgDpTIriaiSNaBONhLRJKUW2mQn5m6vTyqDITmKxZgyOZmqGvWIZ/OFQurL1ZpGJGSa1obb3d6mJAZsHTQBR/IUdjuBcHgGLry6MEnPws2zLcC9P4KlBlUf/vMVFr0/4fbvL8Wjf1ClDS2qO1moAAAAJXRFWHRkYXRlOmNyZWF0ZQAyMDIyLTA1LTI3VDEyOjM3OjU0KzAwOjAwD0FJSAAAACV0RVh0ZGF0ZTptb2RpZnkAMjAyMi0wNS0yN1QxMjozNzo1NCswMDowMH4c8fQAAAAASUVORK5CYII=)
![MSYS2 MinGW UCRT 64 bit](https://img.shields.io/badge/MSYS2-MinGW%20UCRT%2064-yellow.svg?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAMAAAAoLQ9TAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAA2FBMVEWLexaPgB60qmizqGWMfBiOfxypnVKjlkaThCb7+vf///++tX2LfBff2r7b1ralmUrl4curoFfVz6vW0KyMfRne2r3Ry6OVhin8+/n39u+cjzmdkDv//v7OyJ3CuoW/tn6xpmPRyqLx7+Pz8ufSzKWQgB/19Ovn5M/7+/jAt4Dt6trj38e1q2uPfx3+/fz+/v36+fSSgyXm48708ujz8eaOfhvq59TNx5u8s3nCuYTIwZH9/PqZizLx7+SNfRr6+vbUzqiRgiOSgySvpF+NfhuilUWXiC6toltSFs3gAAAAAWJLR0QKaND0VgAAAAd0SU1FB+YFGwwlOIranMwAAACbSURBVBjTjc9HFoIADATQscNg72JHrIAFRVGxt/vfSNoByCZv/mJeAkSaWDyRdFcqnQlBECkB2RzzIRTIIlAiyyFUSLGKGlkPcqPZIuV2h+wG0GOfHAyVEVU/jydTmVRn8wU1HyTqxpKr9cakKLjZ2O4s7MmD1227YPMInHh2oJMXA9eLaQDO7Q5YGpUHnq+3V/T5erf/hAiP/gGUaw3YCIkmVAAAACV0RVh0ZGF0ZTpjcmVhdGUAMjAyMi0wNS0yN1QxMjozNzo1NSswMDowMKk2QvwAAAAldEVYdGRhdGU6bW9kaWZ5ADIwMjItMDUtMjdUMTI6Mzc6NTUrMDA6MDDYa/pAAAAAAElFTkSuQmCC)
![CMake 3.14+](https://img.shields.io/badge/CMake-3.14+-yellow.svg?logo=cmake&logoColor=white)
![C++ 03+](https://img.shields.io/badge/C++-03+-white.svg?logo=cplusplus&logoColor=blue)

It is a library that helps to more easily implement tasks such as service program creation and process creation for each user account in Windows environment.

It is implemented with outdated syntax and features because it is also designed for use in legacy development environments (Visual Studio 2008 SP1).

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

- **Headers :** Win32Ex.h

### System Services

- **Headers :** Win32Ex/System.h
- [More](docs/system-services.md)

#### Processes and Threads

- **Headers :** Win32Ex/System/Process.h, Win32Ex/System/Process.hpp
- [More](docs/procthread.md)

#### Remote Desktop Services

- **Headers :** Win32Ex/System/Session.hpp
- [More](docs/termserv.md)

#### Services

- **Headers :** Win32Ex/System/Service.hpp
- [More](docs/services.md)

#### Windows System Information

- [More](docs/sysinfo.md)

##### Handles and Objects

- **Headers :** Win32Ex/System/Object.h
- [More](docs/sysinfo/handles-and-objects.md)

### Security and Identity

- **Headers :** Win32Ex/Security.h
- [More](docs/secauthz.md)

#### Privileges

- **Headers :** Win32Ex/Security/Privilege.h, Win32Ex/Security/Privilege.hpp
- [More](docs/secauthz/privileges.md)

#### Access Tokens

- **Headers :** Win32Ex/Security/Token.h, Win32Ex/Security/Token.hpp
- [More](docs/secauthz/access-tokens.md)

#### Security Descriptors

- **Headers :** Win32Ex/Security/Descriptor.h
- [More](docs/secauthz/security-descriptors.md)

#### Security Identifiers

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
CPMAddPackage("gh:ntoskrnl7/win32-ex@0.8.21")

# link dependencies
target_link_libraries(tests win32ex)
```

### test.cpp

```C++
#include <iostream>

#include <Win32Ex/System/Process.hpp>
#include <Win32Ex/System/Service.hpp>
// or #include <Win32Ex.h>
// or #include <Win32Ex/System.h>

Win32Ex::System::Service SimpleService("SimpleSvc");

int main()
{
    std::cout << Win32Ex::System::Process::All().size();

    Win32Ex::System::Service::Instance<SimpleService>::Get()
        .OnStart([]() {
            // TODO
        })
        .OnStop([]() {
            // TODO
        })
        .OnPause([]() {
            // TODO
        })
        .OnContinue([]() {
            // TODO
        })
        .OnError([](DWORD ErrorCode, PCSTR Message) {
            // TODO
        })
        .Run();
    return 0;
}
```
