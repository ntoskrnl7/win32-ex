/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Token.hpp

Abstract:

    This Module implements the Token class.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SECURITY_TOKEN_HPP_
#define _WIN32EX_SECURITY_TOKEN_HPP_

#include "../Internal/version.h"
#define WIN32EX_SECURITY_TOKEN_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SECURITY_TOKEN_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SECURITY_TOKEN_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "../Internal/misc.hpp"
#include "Privilege.hpp"
#include "Token.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <functional>

namespace Win32Ex
{
namespace Security
{
class Token
{
    WIN32EX_MOVE_ALWAYS_CLASS(Token)

  private:
    void Move(Token &To)
    {
        To.autoClose_ = autoClose_;
        To.tokenHandle_ = tokenHandle_;
        To.groups_ = groups_;

        autoClose_ = true;
        tokenHandle_ = NULL;
        groups_ = NULL;
    }

  public:
    Token(HANDLE TokenHandle, bool autoClose = true) : autoClose_(autoClose), tokenHandle_(TokenHandle), groups_(NULL)
    {
    }

    Token(std::function<BOOL(DWORD, HANDLE)> condition) : autoClose_(true), groups_(NULL)
    {
        tokenHandle_ = LookupToken2(MAXIMUM_ALLOWED, condition);
    }

    ~Token()
    {
        if (IsValid() && autoClose_)
            CloseHandle(tokenHandle_);

        if (groups_)
            FreeTokenGroups(groups_);
    }

    bool IsValid() const
    {
        return tokenHandle_ != NULL;
    }

    operator HANDLE() const
    {
        return tokenHandle_;
    }

    TokenPrivileges AdjustPrivileges(const std::vector<LUID> &Privileges) const
    {
        return TokenPrivileges(Privileges, tokenHandle_);
    }

    std::vector<LUID_AND_ATTRIBUTES> GetPrivileges() const
    {
        std::vector<LUID_AND_ATTRIBUTES> luids;
        PTOKEN_PRIVILEGES privs = GetTokenPrivileges(tokenHandle_);
        if (!privs)
            return std::vector<LUID_AND_ATTRIBUTES>();

        for (DWORD i = 0; i < privs->PrivilegeCount; ++i)
            luids.push_back(privs->Privileges[i]);

        return luids;
    }

    bool IsAcquired(const LUID &Privilege) const
    {
        PRIVILEGE_SET set;
        set.Control = PRIVILEGE_SET_ALL_NECESSARY;
        set.PrivilegeCount = 1;
        set.Privilege[0].Luid = Privilege;
        set.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;

        BOOL result;
        PrivilegeCheck(tokenHandle_, &set, &result);
        return result == TRUE;
    }

    bool IsAcquired(const std::vector<LUID> &Privileges) const
    {
        PPRIVILEGE_SET set =
            (PPRIVILEGE_SET)malloc(sizeof(PRIVILEGE_SET) + (sizeof(LUID_AND_ATTRIBUTES) * (Privileges.size() - 1)));
        if (!set)
            return false;

        set->Control = PRIVILEGE_SET_ALL_NECESSARY;
        set->PrivilegeCount = (DWORD)Privileges.size();
        PLUID_AND_ATTRIBUTES priv = set->Privilege;
        for (std::vector<LUID>::const_iterator it = Privileges.begin(); it != Privileges.end(); ++it, ++priv)
        {
            priv->Luid = *it;
            priv->Attributes = SE_PRIVILEGE_ENABLED;
        }

        BOOL result;
        PrivilegeCheck(tokenHandle_, set, &result);
        free(set);
        return result == TRUE;
    }

    std::vector<SID_AND_ATTRIBUTES> GetGroups() const
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
    HANDLE tokenHandle_;
    mutable PTOKEN_GROUPS groups_;
};
} // namespace Security
} // namespace Win32Ex

#endif // _WIN32EX_SECURITY_TOKEN_HPP_