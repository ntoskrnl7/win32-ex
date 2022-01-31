
# TlHelp32

- **Headers :** Win32Ex/T/tlhelp32.hpp

## Contents

- [TlHelp32](#tlhelp32)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Functions](#functions)
      - [Process32FirstT\<_CharType\>](#process32firstt_chartype)
      - [Process32NextT<\_CharType\>](#process32nextt_chartype)
    - [Structures](#structures)

## Reference

### Functions

#### Process32FirstT\<_CharType\>

- Example

  ```C++
  #include <Win32Ex/T/tlhelp32.hpp>

  typename PROCESSENTRY32T<CHAR>::Type pe32 = {
      0,
  };
  pe32.dwSize = sizeof(pe32);

  hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (!Process32FirstT<CHAR>(hSnapshot, &pe32))
      return;
  do
  {
    ...
  } while (Process32NextT<CHAR>(hSnapshot, &pe32));
  ...

  CloseHandle(hSnapshot);

  ```

#### Process32NextT<\_CharType\>

### Structures

- PROCESSENTRY32T\<_CharType\>
