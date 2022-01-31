# Result

- **Headers :** Win32Ex/Result.hpp

## Contents

- [Result](#result)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Classes](#classes)
      - [Result\<T\>](#resultt)
      - [Result\<T &\>](#resultt-)
    - [Example](#example)

## Reference

### Classes

#### Result\<T\>

#### Result\<T &\>

### Example

```C++

#include <Win32Ex/Result.hpp>

using namespace Win32Ex;

Result<int> GetValue(bool error)
{
    if (error)
        return Error(ERROR_INVALID_PARAMETER, "Invalid parameter");
    return 1;
}

...

int value = 0;
try
{
    value = GetValue(true).Get();
}
catch (const Error &e)
{
    e.ErrorCode; // == ERROR_INVALID_PARAMETER
    e.what();      // == "Invalid parameter"
}
catch (const std::exception &e)
{
    e.what(); // == "Invalid parameter"
}
value = GetValue(true).Get(-1); // == -1
value = GetValue(false).Get(); // == 1

```
