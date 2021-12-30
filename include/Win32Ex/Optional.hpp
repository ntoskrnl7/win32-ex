/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Optional.hpp

Abstract:

    This Module implements the C++ helper features.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_OPTIONAL_HPP_
#define _WIN32EX_OPTIONAL_HPP_

#include "Internal/version.h"
#define WIN32EX_OPTIONAL_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_OPTIONAL_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_OPTIONAL_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include "Internal/misc.hpp"

namespace Win32Ex
{
template <typename T, typename SFINAE = void> class Optional;

struct None
{
};

namespace Details
{
template <class _StringType> class OptionalConstStringRef
{
    friend class Optional<const _StringType &>;

    WIN32EX_MOVE_ALWAYS_CLASS(OptionalConstStringRef)

  public:
    OptionalConstStringRef Clone() const
    {
        OptionalConstStringRef clone;
        clone.Value_ = Value_;
        clone.Ref_ = Ref_;
        clone.IsNone_ = IsNone_;
        return clone;
    }

  protected:
    void Move(OptionalConstStringRef &To)
    {
        To.Value_ = Value_;
        To.Ref_ = Ref_;
        To.IsNone_ = IsNone_;
        IsNone_ = true;
    }

  public:
    typedef _StringType Type;
    typedef const Type &ConstTypeRefType;
    typedef const Type *ConstTypePtrType;

    typedef typename _StringType::value_type CharType;
    typedef const CharType *ConstPtrType;

    OptionalConstStringRef() : IsNone_(true), Ref_(NULL), Value_(NULL)
    {
    }

    OptionalConstStringRef(None) : IsNone_(true), Ref_(NULL), Value_(NULL)
    {
    }

    OptionalConstStringRef(ConstTypeRefType Value) : IsNone_(false), Ref_(&Value), Value_(NULL)
    {
    }

    OptionalConstStringRef(ConstPtrType Value) : IsNone_(false), Ref_(NULL), Value_(NULL)
    {
        if (Value)
        {
            Value_ = Value;
            Ref_ = new Type(Value_);
        }
    }

    ~OptionalConstStringRef()
    {
        if (Value_ && Ref_)
            delete Ref_;
    }

    ConstPtrType Get(const Type &Default) const
    {
        if (IsNone_)
            return Default.c_str();

        return Get();
    }

    ConstPtrType Get() const
    {
        if (IsNone_)
            throw Exception();

        if (Ref_)
            return Ref_->c_str();

        return Value_;
    }

    operator ConstTypeRefType() const
    {
        if (IsNone_)
            throw Exception();

        if (!Ref_)
            throw NullException();

        return *Ref_;
    }

    bool IsSome() const
    {
        return !IsNone_;
    }

    bool IsNone() const
    {
        return IsNone_;
    }

    bool IsNull() const
    {
        return Ref_ == NULL;
    }

  private:
    ConstPtrType Value_;
    const Type *Ref_;
    bool IsNone_;
};

template <class _StringType> class OptionalString
{
    friend class Optional<_StringType>;

    WIN32EX_MOVE_ALWAYS_CLASS(OptionalString)

  public:
    OptionalString Clone() const
    {
        OptionalString clone;
        clone.Value_ = Value_;
        clone.IsNull_ = IsNull_;
        clone.IsNone_ = IsNone_;
        return clone;
    }

  protected:
    void Move(OptionalString &To)
    {
        To.Value_ = Value_;
        To.IsNull_ = IsNull_;
        To.IsNone_ = IsNone_;
        IsNone_ = true;
    }

  public:
    typedef _StringType Type;
    typedef const Type &ConstRefType;

    typedef const Optional<const Type &> &ConstOptionalRefType;

    typedef typename _StringType::value_type CharType;
    typedef const CharType *ConstPtrType;

    OptionalString() : IsNone_(true)
    {
    }

    OptionalString(None) : IsNone_(true)
    {
    }

    OptionalString(ConstOptionalRefType Value) : IsNone_(false)
    {
        IsNone_ = Value.IsNone();
        if (IsNone_)
            return;

        IsNull_ = Value.IsNull();
        if (!IsNull_)
            Value_ = Value;
    }

    OptionalString(ConstPtrType Value) : IsNone_(false)
    {
        IsNull_ = Value == NULL;
        if (!IsNull_)
            Value_ = Value;
    }

    OptionalString(ConstRefType Value) : IsNone_(false), Value_(Value), IsNull_(false)
    {
    }

    operator ConstRefType() const
    {
        if (IsNone_)
            throw Exception();

        if (IsNull_)
            throw NullException();

        return Value_;
    }

    operator Type &()
    {
        if (IsNone_)
            throw Exception();

        if (IsNull_)
            throw NullException();

        return Value_;
    }

    ConstPtrType Get(ConstRefType Default) const
    {
        if (IsNone_)
            return Default.c_str();

        return Get();
    }

    ConstPtrType Get() const
    {
        if (IsNone_)
            throw Exception();

        return IsNull_ ? NULL : Value_.c_str();
    }

    bool IsSome() const
    {
        return !IsNone_;
    }

    bool IsNone() const
    {
        return IsNone_;
    }

    bool IsNull() const
    {
        return IsNull_;
    }

  private:
    typename std::remove_const<Type>::type Value_;
    bool IsNull_;
    bool IsNone_;
};
} // namespace Details

template <> class Optional<const String &> : public Details::OptionalConstStringRef<String>
{
    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    Optional Clone() const
    {
        Optional clone(OptionalConstStringRef<String>::Clone());
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        OptionalConstStringRef<String>::Move(To);
    }

  public:
    Optional() : OptionalConstStringRef()
    {
    }

    Optional(None none) : OptionalConstStringRef(none)
    {
    }

    Optional(OptionalConstStringRef::ConstTypeRefType Value) : OptionalConstStringRef(Value)
    {
    }

    Optional(OptionalConstStringRef::ConstPtrType Value) : OptionalConstStringRef(Value)
    {
    }
};

template <> class Optional<const StringW &> : public Details::OptionalConstStringRef<StringW>
{
    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    Optional Clone() const
    {
        Optional clone(OptionalConstStringRef<StringW>::Clone());
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        OptionalConstStringRef<StringW>::Move(To);
    }

  public:
    Optional() : OptionalConstStringRef()
    {
    }

    Optional(None none) : OptionalConstStringRef(none)
    {
    }

    Optional(OptionalConstStringRef::ConstTypeRefType Value) : OptionalConstStringRef(Value)
    {
    }

    Optional(OptionalConstStringRef::ConstPtrType Value) : OptionalConstStringRef(Value)
    {
    }
};

template <> class Optional<String> : public Details::OptionalString<String>
{
    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    Optional Clone() const
    {
        Optional clone(OptionalString<String>::Clone());
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        OptionalString<String>::Move(To);
    }

  public:
    Optional(Details::OptionalString<String> &Other) : OptionalString(Other)
    {
    }

    Optional() : OptionalString()
    {
    }

    Optional(None none) : OptionalString(none)
    {
    }

    Optional(OptionalString::ConstOptionalRefType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstPtrType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstRefType Value) : OptionalString(Value)
    {
    }
};

template <> class Optional<StringW> : public Details::OptionalString<StringW>
{
    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    Optional Clone() const
    {
        Optional clone(OptionalString<StringW>::Clone());
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        OptionalString<StringW>::Move(To);
    }

  public:
    Optional() : OptionalString()
    {
    }

    Optional(None none) : OptionalString(none)
    {
    }

    Optional(OptionalString::ConstOptionalRefType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstPtrType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstRefType Value) : OptionalString(Value)
    {
    }
};

template <> class Optional<const String> : public Details::OptionalString<const String>
{
  public:
    Optional() : OptionalString()
    {
    }

    Optional(None none) : OptionalString(none)
    {
    }

    Optional(OptionalString::ConstOptionalRefType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstPtrType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstRefType Value) : OptionalString(Value)
    {
    }
};

template <> class Optional<const StringW> : public Details::OptionalString<const StringW>
{
  public:
    Optional() : OptionalString()
    {
    }

    Optional(None none) : OptionalString(none)
    {
    }

    Optional(OptionalString::ConstOptionalRefType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstPtrType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstRefType Value) : OptionalString(Value)
    {
    }
};

template <typename T> class Optional<T, typename std::enable_if<std::is_reference<T>::value>::type>
{
    WIN32EX_MOVE_ALWAYS_CLASS_EX(Optional, Value_(Other.Value_))

  public:
    Optional Clone() const
    {
        Optional clone;
        clone.Value_ = Value_;
        clone.IsNone_ = IsNone_;
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        To.Value_ = Value_;
        To.IsNone_ = IsNone_;
        IsNone_ = true;
    }

  public:
    typedef T Type;

    Optional() : Value_(NULL), IsNone_(true)
    {
    }

    Optional(None) : Value_(NULL), IsNone_(true)
    {
    }

    Optional(T Value) : Value_(&Value), IsNone_(false)
    {
    }

    T Get() const
    {
        if (IsNone_)
            throw Exception();

        return *Value_;
    }

    operator T() const
    {
        return Get();
    }

    T operator->() const
    {
        if (IsNone_)
            throw Exception();

        return Value_;
    }

    bool IsSome() const
    {
        return !IsNone_;
    }

    bool IsNone() const
    {
        return IsNone_;
    }

  private:
    typename std::remove_reference<T>::type *Value_;
    bool IsNone_;
};

template <typename T> class Optional<T, typename std::enable_if<!std::is_reference<T>::value>::type>
{
    WIN32EX_MOVE_ALWAYS_CLASS_EX(Optional, Value_(Other.Value_))

  public:
    Optional Clone() const
    {
        Optional clone;
        clone.Value_ = Value_;
        clone.IsNone_ = IsNone_;
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        To.Value_ = Value_;
        To.IsNone_ = IsNone_;
        IsNone_ = true;
    }

  public:
    typedef T Type;

    Optional() : IsNone_(true)
    {
    }

    Optional(None) : IsNone_(true)
    {
    }

    Optional(T Value) : Value_(Value), IsNone_(false)
    {
    }

    T Get() const
    {
        if (IsNone_)
            throw Exception();

        return Value_;
    }

    operator T() const
    {
        if (IsNone_)
            throw Exception();

        return Value_;
    }

    operator typename std::remove_reference<T>::type &()
    {
        if (IsNone_)
            throw Exception();

        return Value_;
    }

    T operator->() const
    {
        if (IsNone_)
            throw Exception();

        return Value_;
    }

    bool IsSome() const
    {
        return !IsNone_;
    }

    bool IsNone() const
    {
        return IsNone_;
    }

  private:
    typename std::remove_const<T>::type Value_;
    bool IsNone_;
};

#if defined(__cpp_variadic_templates)
template <typename... Args> bool IsAll(const Optional<Args> &...args)
{
    for (auto val : {
             args.IsNone()...,
         })
    {
        if (val)
        {
            return false;
        }
    }
    return true;
}

template <typename... Args> bool IsSome(const Optional<Args> &...args)
{
    for (auto val : {
             args.IsSome()...,
         })
    {
        if (val)
        {
            return true;
        }
    }
    return false;
}

template <typename... Args> bool IsAny(const Optional<Args> &...args)
{
    return IsSome(args...);
}

template <typename... Args> bool IsNone(const Optional<Args> &...args)
{
    for (auto val : {
             args.IsSome()...,
         })
    {
        if (val)
        {
            return false;
        }
    }
    return true;
}
#endif
} // namespace Win32Ex

#endif // _WIN32EX_OPTIONAL_HPP_
