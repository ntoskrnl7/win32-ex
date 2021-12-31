#include <Win32Ex/System/Object.h>

BOOL MakePermanentObjectC()
{
    HANDLE handle = CreateMutex(NULL, FALSE, TEXT("Test"));
    if (handle != NULL)
    {
        if (IsUserAdmin(NULL))
        {
            if (IsPermanentObject(handle))
                return FALSE;
            if (!MakePermanentObject(handle))
                return FALSE;
            if (!IsPermanentObject(handle))
                return FALSE;
            if (IsTemporaryObject(handle))
                return FALSE;
        }
        if (!MakeTemporaryObject(handle))
            return FALSE;
        if (IsPermanentObject(handle))
            return FALSE;
        if (!IsTemporaryObject(handle))
            return FALSE;
        CloseHandle(handle);
        return TRUE;
    }
    return FALSE;
}
