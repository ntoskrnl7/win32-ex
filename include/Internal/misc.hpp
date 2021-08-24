#pragma once
#include "misc.h"
#include <string>
#include <tchar.h>

namespace Win32Ex
{
typedef std::basic_string<TCHAR> TString;
}

#ifndef __cpp_lambdas
#if defined(_MSC_VER) && _MSC_VER >= 1600
#define __cpp_lambdas 200907L
#endif
#endif