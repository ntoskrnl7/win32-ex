# Processes and Threads

- **Headers :** Win32Ex/T/processthreadsapi.hpp

## Contents

- [Processes and Threads](#processes-and-threads)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Functions](#functions)
      - [CreateProcessT\<CharType\>](#createprocesst_chartype)
      - [CreateProcessAsUserT\<CharType\>](#createprocessasusert_chartype)
      - [QueryFullProcessImageNameT\<CharType\>](#queryfullprocessimagenamet_chartype)
    - [Structures](#structures)

## Reference

### Functions

#### CreateProcessT\<CharType\>

#### CreateProcessAsUserT\<CharType\>

- Example

    ```C++
    #include <Win32Ex/T/processthreadsapi.hpp>

    STARTUPINFOT<CHAR> si;

    ...

    Win32Ex::CreateProcessAsUserT<CHAR>(..., &si, ..);
    ```

#### QueryFullProcessImageNameT\<CharType\>

### Structures

- STARTUPINFOT\<CharType\>
- STARTUPINFOEXT\<CharType\>
