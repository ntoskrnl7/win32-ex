# Processes and Threads

- **Headers :** Win32Ex/T/processthreadsapi.hpp

## Contents

- [Processes and Threads](#processes-and-threads)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Functions](#functions)
      - [CreateProcessT\<_CharType\>](#createprocesst_chartype)
      - [CreateProcessAsUserT\<_CharType\>](#createprocessasusert_chartype)
      - [QueryFullProcessImageNameT\<_CharType\>](#queryfullprocessimagenamet_chartype)
    - [Structures](#structures)

## Reference

### Functions

#### CreateProcessT\<_CharType\>

#### CreateProcessAsUserT\<_CharType\>

- Example

    ```C++
    #include <Win32Ex/T/processthreadsapi.hpp>

    STARTUPINFOT<CHAR> si;

    ...

    Win32Ex::CreateProcessAsUserT<CHAR>(..., &si, ..);
    ```

#### QueryFullProcessImageNameT\<_CharType\>

### Structures

- STARTUPINFOT\<_CharType\>
- STARTUPINFOEXT\<_CharType\>
