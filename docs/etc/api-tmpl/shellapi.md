# ShellApi

- **Headers :** Win32Ex/T/shellapi.hpp

## Contents

- [ShellApi](#shellapi)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Functions](#functions)
      - [ShellExecuteExT\<_CharType\>](#shellexecuteext_chartype)
    - [Structures](#structures)
      - [SHELLEXECUTEINFOT\<_CharType\>](#shellexecuteinfot_chartype)

## Reference

### Functions

#### ShellExecuteExT\<_CharType\>

- Example

  ```C++
  #include <Win32Ex/T/shellapi.hpp>

  SHELLEXECUTEINFOT<CHAR> sei;
  ZeroMemory(&sei, sizeof(sei));
  // or
  // typename SHELLEXECUTEINFOT<CHAR>::Type sei = { 0, };

  ...

  Win32Ex::ShellExecuteExT<CHAR>(&sei);
  ```

### Structures

#### SHELLEXECUTEINFOT\<_CharType\>
