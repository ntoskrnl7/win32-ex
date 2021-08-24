#pragma once

#include "..\Internal\misc.hpp"
#include "Privilege.hpp"
#include "Token.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <functional>

namespace Win32Ex
{
namespace Security
{
class Token
{
  public:
    Token(HANDLE TokenHandle, bool autoClose = true) : autoClose_(autoClose), tokenHandle_(TokenHandle), groups_(NULL)
    {
    }

    Token(std::function<BOOL(DWORD, HANDLE)> condition) : autoClose_(true), groups_(NULL)
    {
        tokenHandle_ = LookupToken2(MAXIMUM_ALLOWED, condition);
    }
#ifdef __cpp_rvalue_references
    Token(Token &&Other) noexcept
    {
        tokenHandle_ = Other.tokenHandle_;
        Other.tokenHandle_ = NULL;
        groups_ = Other.groups_;
        Other.groups_ = NULL;
        autoClose_ = Other.autoClose_;
        Other.autoClose_ = false;
    }
#endif
    ~Token()
    {
        if (IsValid() && autoClose_)
            CloseHandle(tokenHandle_);
        if (groups_)
            FreeTokenGroups(groups_);
    }

    BOOL IsValid()
    {
        return tokenHandle_ != NULL;
    }

    operator HANDLE()
    {
        return tokenHandle_;
    }

    TokenPrivileges AdjustPrivileges(const std::vector<LUID> &Privileges)
    {
        return TokenPrivileges(Privileges, tokenHandle_);
    }

    std::vector<LUID_AND_ATTRIBUTES> GetPrivileges()
    {
        std::vector<LUID_AND_ATTRIBUTES> luids;
        PTOKEN_PRIVILEGES privs = GetTokenPrivileges(tokenHandle_);
        if (!privs)
            return std::vector<LUID_AND_ATTRIBUTES>();
        for (DWORD i = 0; i < privs->PrivilegeCount; ++i)
            luids.push_back(privs->Privileges[i]);
        return luids;
    }

    class Group
    {
      public:
        Group(SID_AND_ATTRIBUTES group)
        {
        }
        ~Group()
        {
            if (sid_)
                FreeTokenUserSid(sid_);
        }

      private:
        PSID sid_;
    };

    std::vector<SID_AND_ATTRIBUTES> GetGroups()
    {
        std::vector<SID_AND_ATTRIBUTES> ret;
        if (groups_)
            FreeTokenGroups(groups_);
        groups_ = GetTokenGroups(tokenHandle_);
        if (groups_)
            for (DWORD i = 0; i < groups_->GroupCount; ++i)
                ret.push_back(groups_->Groups[i]);
        return ret;
    }

  public:
    static Token Lookup(std::function<bool(HANDLE)> condition)
    {
        return Token(LookupToken(MAXIMUM_ALLOWED, condition));
    }

    static Token Lookup(DWORD DesireAccess, std::function<bool(HANDLE)> condition)
    {
        return Token(LookupToken(DesireAccess, condition));
    }

    static Token Lookup(std::function<bool(HANDLE, PVOID)> condition, _In_opt_ PVOID Context = NULL)
    {
        return Token(LookupTokenEx(MAXIMUM_ALLOWED, condition, Context));
    }

    static Token Lookup(DWORD DesireAccess, std::function<bool(HANDLE, PVOID)> condition, _In_opt_ PVOID Context = NULL)
    {
        return Token(LookupTokenEx(DesireAccess, condition, Context));
    }

    static Token Lookup(std::function<bool(DWORD, HANDLE)> condition)
    {
        return Token(LookupToken2(MAXIMUM_ALLOWED, condition));
    }

    static Token Lookup(DWORD DesireAccess, std::function<bool(DWORD, HANDLE)> condition)
    {
        return Token(LookupToken2(DesireAccess, condition));
    }

    static Token Lookup(std::function<bool(DWORD, HANDLE, PVOID)> condition, _In_opt_ PVOID Context = NULL)
    {
        return Token(LookupTokenEx2(MAXIMUM_ALLOWED, condition, Context));
    }

    static Token Lookup(DWORD DesireAccess, std::function<bool(DWORD, HANDLE, PVOID)> condition,
                        _In_opt_ PVOID Context = NULL)
    {
        return Token(LookupTokenEx2(DesireAccess, condition, Context));
    }

    static Token Current()
    {
        HANDLE handle;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &handle))
            return Token(handle);
        return Token(NULL);
    }

  private:
    bool autoClose_;
    PTOKEN_GROUPS groups_;
    HANDLE tokenHandle_;
};
} // namespace Security
} // namespace Win32Ex