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
template <class _StringType> class OptionalString;

template <class _StringType> class OptionalConstStringRef
{
    friend class Optional<const _StringType &>;
    friend class OptionalString<_StringType>;

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

    void Move(OptionalString<_StringType> &To)
    {
        To.IsNone_ = IsNone_;
        if (To.IsNone_)
            return;

        To.IsNull_ = Ref_ == NULL;
        if (!To.IsNull_)
            To.Value_ = *Ref_;

        Clear_();
        Value_ = NULL;
        Ref_ = NULL;
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

    OptionalConstStringRef(const Optional<Type> &Other)
        : IsNone_(Other.IsNone_), Ref_(Other.IsNull_ ? NULL : &Other.Value_), Value_(NULL)
    {
    }

    OptionalConstStringRef(Optional<Type> &Other) : IsNone_(false), Ref_(NULL), Value_(NULL)
    {
        Other.Move(*this);
    }

    ~OptionalConstStringRef()
    {
        Clear_();
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
    void Clear_()
    {
        if (Value_ && Ref_)
            delete Ref_;
    }

  private:
    ConstPtrType Value_;
    const Type *Ref_;
    bool IsNone_;
};

template <class _StringType> class OptionalString
{
    friend class Optional<_StringType>;
    friend class OptionalConstStringRef<_StringType>;

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
        To.Value_.swap(Value_);
        Value_.clear();
        IsNone_ = true;
    }

    void Move(OptionalConstStringRef<_StringType> &To)
    {
        To.IsNone_ = IsNone_;
        if (To.IsNone_)
            return;
        To.Ref_ = (IsNull_) ? NULL : new _StringType(Value_);
        To.Value_ = NULL;
        Value_.clear();
        IsNone_ = true;
    }

  public:
    typedef _StringType Type;
    typedef const Type &ConstRefType;

    typedef typename _StringType::value_type CharType;
    typedef const CharType *ConstPtrType;

    OptionalString() : IsNone_(true)
    {
    }

    OptionalString(None) : IsNone_(true)
    {
    }

    OptionalString(Optional<const Type &> &Other) : IsNone_(false)
    {
        Other.Move(*this);
    }

    OptionalString(const Optional<const Type &> &Other) : IsNone_(false)
    {
        IsNone_ = Other.IsNone();
        if (IsNone_)
            return;

        IsNull_ = Other.IsNull();
        if (!IsNull_)
            Value_ = Other;
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
  public:
    typedef OptionalConstStringRef::Type Type;

    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    Optional Clone() const
    {
        Optional clone(OptionalConstStringRef<Type>::Clone());
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        OptionalConstStringRef<Type>::Move(To);
    }

  public:
    friend class Details::OptionalString<Type>;

    void Move(Details::OptionalString<Type> &To)
    {
        OptionalConstStringRef<Type>::Move(To);
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

    Optional(const Optional<Type> &Other) : OptionalConstStringRef(Other)
    {
    }

    Optional(Optional<Type> &Other) : OptionalConstStringRef(Other)
    {
    }

#if !defined(__cpp_rvalue_references)
    Optional &operator=(OptionalConstStringRef::ConstTypeRefType Rhs)
    {
        OptionalConstStringRef(Rhs).Move(*this);
        return *this;
    }

    Optional &operator=(OptionalConstStringRef::ConstPtrType Rhs)
    {
        OptionalConstStringRef(Rhs).Move(*this);
        return *this;
    }

    Optional &operator=(Optional<Type> &Rhs)
    {
        OptionalConstStringRef(Rhs).Move(*this);
        return *this;
    }
#endif
};

template <> class Optional<const StringW &> : public Details::OptionalConstStringRef<StringW>
{
  public:
    typedef OptionalConstStringRef::Type Type;

    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    Optional Clone() const
    {
        Optional clone(OptionalConstStringRef<Type>::Clone());
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        OptionalConstStringRef<Type>::Move(To);
    }

  protected:
    friend class Details::OptionalString<Type>;

    void Move(Details::OptionalString<Type> &To)
    {
        OptionalConstStringRef<Type>::Move(To);
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

    Optional(const Optional<Type> &Other) : OptionalConstStringRef(Other)
    {
    }

    Optional(Optional<Type> &Other) : OptionalConstStringRef(Other)
    {
    }

#if !defined(__cpp_rvalue_references)
    Optional &operator=(OptionalConstStringRef::ConstTypeRefType Rhs)
    {
        OptionalConstStringRef(Rhs).Move(*this);
        return *this;
    }

    Optional &operator=(OptionalConstStringRef::ConstPtrType Rhs)
    {
        OptionalConstStringRef(Rhs).Move(*this);
        return *this;
    }

    Optional &operator=(Optional<Type> &Rhs)
    {
        OptionalConstStringRef(Rhs).Move(*this);
        return *this;
    }
#endif
};

template <> class Optional<String> : public Details::OptionalString<String>
{
  public:
    typedef OptionalString::Type Type;

    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    Optional Clone() const
    {
        Optional clone(OptionalString<Type>::Clone());
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        OptionalString<Type>::Move(To);
    }

  protected:
    friend class Details::OptionalConstStringRef<Type>;

    void Move(Details::OptionalConstStringRef<Type> &To)
    {
        OptionalString<Type>::Move(To);
    }

  public:
    Optional(Details::OptionalString<Type> &Other) : OptionalString(Other)
    {
    }

    Optional() : OptionalString()
    {
    }

    Optional(None none) : OptionalString(none)
    {
    }

    Optional(const Optional<const Type &> &Value) : OptionalString(Value)
    {
    }

    Optional(Optional<const Type &> &Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstPtrType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstRefType Value) : OptionalString(Value)
    {
    }

#if !defined(__cpp_rvalue_references)
    Optional &operator=(Optional<const Type &> &Rhs)
    {
        OptionalString(Rhs).Move(*this);
        return *this;
    }

    Optional &operator=(OptionalString::ConstPtrType Rhs)
    {
        OptionalString(Rhs).Move(*this);
        return *this;
    }
#endif
};

template <> class Optional<StringW> : public Details::OptionalString<StringW>
{
  public:
    typedef OptionalString::Type Type;

    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    Optional Clone() const
    {
        Optional clone(OptionalString<Type>::Clone());
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        OptionalString<Type>::Move(To);
    }

  protected:
    friend class Details::OptionalConstStringRef<Type>;

    void Move(Details::OptionalConstStringRef<Type> &To)
    {
        OptionalString<Type>::Move(To);
    }

  public:
    Optional(Details::OptionalString<Type> &Other) : OptionalString(Other)
    {
    }

    Optional() : OptionalString()
    {
    }

    Optional(None none) : OptionalString(none)
    {
    }

    Optional(const Optional<const Type &> &Value) : OptionalString(Value)
    {
    }

    Optional(Optional<const Type &> &Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstPtrType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstRefType Value) : OptionalString(Value)
    {
    }

#if !defined(__cpp_rvalue_references)
    Optional &operator=(Optional<const Type &> &Rhs)
    {
        OptionalString(Rhs).Move(*this);
        return *this;
    }

    Optional &operator=(OptionalString::ConstPtrType Rhs)
    {
        OptionalString(Rhs).Move(*this);
        return *this;
    }
#endif
};

template <> class Optional<const String> : public Details::OptionalString<const String>
{
    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    Optional Clone() const
    {
        Optional clone(OptionalString<Type>::Clone());
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        OptionalString<Type>::Move(To);
    }

  public:
    Optional() : OptionalString()
    {
    }

    Optional(None none) : OptionalString(none)
    {
    }

    Optional(const Optional<const Type &> &Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstPtrType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstRefType Value) : OptionalString(Value)
    {
    }

#if !defined(__cpp_rvalue_references)
    Optional &operator=(Optional<const Type &> &Rhs)
    {
        OptionalString(Rhs).Move(*this);
        return *this;
    }

    Optional &operator=(OptionalString::ConstPtrType Rhs)
    {
        OptionalString(Rhs).Move(*this);
        return *this;
    }
#endif
};

template <> class Optional<const StringW> : public Details::OptionalString<const StringW>
{
    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    Optional Clone() const
    {
        Optional clone(OptionalString<Type>::Clone());
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        OptionalString<Type>::Move(To);
    }

  public:
    Optional() : OptionalString()
    {
    }

    Optional(None none) : OptionalString(none)
    {
    }

    Optional(const Optional<const Type &> &Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstPtrType Value) : OptionalString(Value)
    {
    }

    Optional(OptionalString::ConstRefType Value) : OptionalString(Value)
    {
    }

#if !defined(__cpp_rvalue_references)
    Optional &operator=(Optional<const Type &> &Rhs)
    {
        OptionalString(Rhs).Move(*this);
        return *this;
    }

    Optional &operator=(OptionalString::ConstPtrType Rhs)
    {
        OptionalString(Rhs).Move(*this);
        return *this;
    }
#endif
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

#if !defined(__cpp_rvalue_references)
    Optional &operator=(T Rhs)
    {
        Optional(Rhs).Move(*this);
        return *this;
    }
#endif

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

#if !defined(__cpp_rvalue_references)
    Optional &operator=(T Rhs)
    {
        Optional(Rhs).Move(*this);
        return *this;
    }
#endif

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
