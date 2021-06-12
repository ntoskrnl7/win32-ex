#include <System/Object.h>
#include <Windows.h>

BOOL MakePermanentObjectTestC()
{
    HANDLE handle = CreateMutex(NULL, FALSE, TEXT("Test"));
    if (handle != NULL)
    {
        if (IsUserAdmin())
        {
            if (IsPermanentObject(handle))
                return FALSE;
            if (!MakePermanentObject(handle))
                return FALSE;
            if (!IsPermanentObject(handle))
                return FALSE;
        }
        if (!MakeTemporaryObject(handle))
            return FALSE;
        if (IsPermanentObject(handle))
            return FALSE;
        CloseHandle(handle);
        return TRUE;
    }
    return FALSE;
}