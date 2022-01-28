# Optional

- **Headers :** Win32Ex/Optional.hpp

## Contents

- [Optional](#optional)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Classes](#classes)
      - [Optional\<T\>](#optionalt)
      - [Optional\<T &\>](#optionalt-)
      - [Optional\<String\>](#optionalstring)
      - [Optional\<StringW\>](#optionalstringw)
      - [Optional\<const String\>](#optionalconst-string)
      - [Optional\<const StringW\>](#optionalconst-stringw)
      - [Optional\<String &\>](#optionalstring-)
      - [Optional\<StringW &\>](#optionalstringw-)
      - [Optional\<const String &\>](#optionalconst-string-)
      - [Optional\<const StringW &\>](#optionalconst-stringw-)
      - [Optional\<const StringT &\>](#optionalconst-stringt-)
    - [Example](#example)

## Reference

### Classes

#### Optional\<T\>

#### Optional\<T &\>

#### Optional\<String\>

#### Optional\<StringW\>

#### Optional\<const String\>

#### Optional\<const StringW\>

#### Optional\<String &\>

#### Optional\<StringW &\>

#### Optional\<const String &\>

#### Optional\<const StringW &\>

#### Optional\<const StringT &\>

### Example

```C++

#include <Win32Ex/Optional.hpp>

using namespace Win32Ex;

void TestFn(Optional<int> arg0 = None(), Optional<double> arg1 = None(), Optional<String> arg2 = None())
{
  if (arg0.IsSome())
  {
    int val = arg0;
  }

  if (arg1.IsSome())
  {
    double val = arg1;
  }

  if (arg2.IsSome())
  {
    String val = arg2;
  }

  if (IsAll(arg0, arg1, arg2))
  {
    int val0 = arg0;
    double val1 = arg1;
    String val2 = arg2;
    ...
  }

  if (IsNone(arg0, arg1, arg2))
  {
    ...
  }
}

TestFn();
TestFn(1);
TestFn(None(), 2, None());
TestFn(None(), None(), "3");
TestFn(None(), 2, "3");
TestFn(1, None(), "3");
TestFn(1, 2);
TestFn(1, 2, "3");
```
