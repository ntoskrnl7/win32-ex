# Handles and Objects

- **Link :** <https://docs.microsoft.com/en-us/windows/win32/sysinfo/handles-and-objects>
- **Headers :** Win32Ex/System/Object.h

## Contents

- [Handles and Objects](#handles-and-objects)
  - [Contents](#contents)
  - [Reference](#reference)
    - [Functions](#functions)
      - [MakePermanentObject](#makepermanentobject)
      - [MakeTemporaryObject](#maketemporaryobject)
      - [IsPermanentObject](#ispermanentobject)
      - [IsTemporaryObject](#istemporaryobject)
    - [Example](#example)

## Reference

### Functions

#### MakePermanentObject

#### MakeTemporaryObject

#### IsPermanentObject

#### IsTemporaryObject

### Example

C/C++

```C
#include <Win32Ex/System/Object.h>

HANDLE handle = ....
MakePermanentObject(handle); // Administrator privilege required
IsPermanentObject(handle); // == TRUE
MakeTemporaryObject(handle);
IsTemporaryObject(handle); // == TRUE
```
