﻿/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Optional.hpp

Abstract:

    This Module implements the Optional classes.

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

  public:
    typedef _StringType Type;
    typedef const Type &ConstTypeRefType;
    typedef const Type *ConstTypePtrType;

    typedef typename _StringType::value_type CharType;
    typedef const CharType *ConstPtrType;

    WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED(OptionalConstStringRef)

  public:
    OptionalConstStringRef Clone() const
    {
        OptionalConstStringRef clone;
        if (IsNone_)
            return clone;
        clone.IsNone_ = IsNone_;
        if (Ref_)
            clone.Ref_ = new Type(*Ref_);
        clone.Value_ = NULL;
        return clone;
    }

  protected:
    void Move(OptionalConstStringRef &To)
    {
        To.IsMoved_ = IsMoved_;
        if (IsMoved_)
            return;

        To.Value_ = Value_;
        To.Ref_ = Ref_;
        To.IsNone_ = IsNone_;
        IsNone_ = true;
        IsMoved_ = true;
    }

    void Move(OptionalString<Type> &To)
    {
        To.IsMoved_ = IsMoved_;
        if (IsMoved_)
            return;

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
        IsMoved_ = true;
    }

    void Move(OptionalString<const Type> &To)
    {
        To.IsMoved_ = IsMoved_;
        if (IsMoved_)
            return;

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
        IsMoved_ = true;
    }

  public:
    OptionalConstStringRef() : IsNone_(true), Ref_(NULL), Value_(NULL), IsMoved_(false)
    {
    }

    OptionalConstStringRef(None) : IsNone_(true), Ref_(NULL), Value_(NULL), IsMoved_(false)
    {
    }

    OptionalConstStringRef(ConstTypeRefType Value) : IsNone_(false), Ref_(&Value), Value_(NULL), IsMoved_(false)
    {
    }

    OptionalConstStringRef(ConstPtrType Value) : IsNone_(false), Ref_(NULL), Value_(NULL), IsMoved_(false)
    {
        if (Value)
        {
            Value_ = Value;
            Ref_ = new Type(Value_);
        }
    }

    OptionalConstStringRef(const Optional<Type> &Other)
        : IsNone_(Other.IsNone_), Ref_(Other.IsNull_ ? NULL : &Other.Value_), Value_(NULL), IsMoved_(Other.IsMoved_)
    {
    }

    OptionalConstStringRef(Optional<Type> &Other) : IsNone_(false), Ref_(NULL), Value_(NULL), IsMoved_(false)
    {
        Other.Move(*this);
    }

    OptionalConstStringRef(Optional<const Type> &Other) : IsNone_(false), Ref_(NULL), Value_(NULL), IsMoved_(false)
    {
        Other.Move(*this);
    }

    ~OptionalConstStringRef()
    {
        Clear_();
    }

    ConstPtrType Get(const Type &Default) const
    {
        if (IsMoved_)
            return Default.c_str();

        if (IsNone_)
            return Default.c_str();

        return Get();
    }

    ConstPtrType Get() const
    {
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        if (Ref_)
            return Ref_->c_str();

        return Value_;
    }

    operator ConstTypeRefType() const
    {
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        if (!Ref_)
            throw NullException();

        return *Ref_;
    }

    bool IsSome() const
    {
        return !IsMoved_ && !IsNone_;
    }

    bool IsNone() const
    {
        return !IsMoved_ && IsNone_;
    }

    bool IsNull() const
    {
        return !IsMoved_ && Ref_ == NULL;
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
    friend class OptionalConstStringRef<typename std::remove_const<_StringType>::type>;

    WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED(OptionalString)

  public:
    OptionalString Clone() const
    {
        if (IsMoved_)
            throw MovedException();

        OptionalString clone;
        clone.Value_ = Value_;
        clone.IsNull_ = IsNull_;
        clone.IsNone_ = IsNone_;
        return clone;
    }

  protected:
    void Move(OptionalString &To)
    {
        To.IsMoved_ = IsMoved_;
        if (IsMoved_)
            return;

        To.Value_ = Value_;
        To.IsNull_ = IsNull_;
        To.IsNone_ = IsNone_;
        To.Value_.swap(Value_);
        Value_.clear();
        IsNone_ = true;
        IsMoved_ = true;
    }

    void Move(OptionalConstStringRef<_StringType> &To)
    {
        To.IsMoved_ = IsMoved_;
        if (IsMoved_)
            return;

        To.IsNone_ = IsNone_;
        if (To.IsNone_)
            return;

        To.Ref_ = (IsNull_) ? NULL : new _StringType(Value_);
        To.Value_ = NULL;
        Value_.clear();
        IsNone_ = true;
        IsMoved_ = true;
    }

  public:
    typedef _StringType Type;
    typedef const Type &ConstRefType;

    typedef typename _StringType::value_type CharType;
    typedef const CharType *ConstPtrType;

    OptionalString() : IsNone_(true), IsMoved_(false)
    {
    }

    OptionalString(None) : IsNone_(true), IsMoved_(false)
    {
    }

    OptionalString(Optional<const typename std::remove_const<Type>::type &> &Other) : IsNone_(false), IsMoved_(false)
    {
        Other.Move(*this);
    }

    OptionalString(const Optional<const typename std::remove_const<Type>::type &> &Other)
        : IsNone_(Other.IsNone()), IsMoved_(Other.IsMoved())
    {
        if (IsNone_)
            return;

        IsNull_ = Other.IsNull();
        if (!IsNull_)
            Value_ = Other;
    }

    OptionalString(ConstPtrType Value) : IsNone_(false), IsMoved_(false)
    {
        IsNull_ = Value == NULL;
        if (!IsNull_)
            Value_ = Value;
    }

    OptionalString(ConstRefType Value) : IsNone_(false), Value_(Value), IsNull_(false), IsMoved_(false)
    {
    }

    operator ConstRefType() const
    {
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        if (IsNull_)
            throw NullException();

        return Value_;
    }

    operator Type &()
    {
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        if (IsNull_)
            throw NullException();

        return Value_;
    }

    ConstPtrType Get(ConstRefType Default) const
    {
        if (IsMoved_)
            throw Default.c_str();

        if (IsNone_)
            return Default.c_str();

        return Get();
    }

    ConstPtrType Get() const
    {
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        return IsNull_ ? NULL : Value_.c_str();
    }

    bool IsSome() const
    {
        return !IsMoved_ && !IsNone_;
    }

    bool IsNone() const
    {
        return !IsMoved_ && IsNone_;
    }

    bool IsNull() const
    {
        return !IsMoved_ && IsNull_;
    }

  protected:
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
    bool IsMoved() const
    {
        return OptionalConstStringRef::IsMoved();
    }

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
    friend class Details::OptionalString<const Type>;

    void Move(Details::OptionalString<Type> &To)
    {
        OptionalConstStringRef<Type>::Move(To);
    }

    void Move(Details::OptionalString<const Type> &To)
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
    bool IsMoved() const
    {
        return OptionalConstStringRef::IsMoved();
    }

    Optional Clone() const
    {
        if (OptionalConstStringRef<Type>::IsMoved())
            throw MovedException();

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
    friend class Details::OptionalString<const Type>;

    void Move(Details::OptionalString<Type> &To)
    {
        OptionalConstStringRef<Type>::Move(To);
    }

    void Move(Details::OptionalString<const Type> &To)
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
    bool IsMoved() const
    {
        return OptionalString::IsMoved();
    }

    Optional Clone() const
    {
        if (OptionalString<Type>::IsMoved())
            throw MovedException();

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

    Optional &operator=(const Optional<const Type &> &Rhs)
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
    bool IsMoved() const
    {
        return OptionalString::IsMoved();
    }

    Optional Clone() const
    {
        if (OptionalString<Type>::IsMoved())
            throw MovedException();

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

    Optional &operator=(const Optional<const Type &> &Rhs)
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
  public:
    typedef OptionalString::Type Type;

    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    bool IsMoved() const
    {
        return OptionalString::IsMoved();
    }

    Optional Clone() const
    {
        if (OptionalString<Type>::IsMoved())
            throw MovedException();

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
  public:
    typedef OptionalString::Type Type;

    WIN32EX_MOVE_ALWAYS_CLASS(Optional)

  public:
    bool IsMoved() const
    {
        return OptionalString::IsMoved();
    }

    Optional Clone() const
    {
        if (OptionalString<Type>::IsMoved())
            throw MovedException();

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
    WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED_EX(Optional, Value_(Other.Value_))

  public:
    Optional Clone() const
    {
        if (IsMoved_)
            throw MovedException();

        Optional clone;
        clone.Value_ = Value_;
        clone.IsNone_ = IsNone_;
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        To.IsMoved_ = IsMoved_;
        if (IsMoved_)
            return;

        To.Value_ = Value_;
        To.IsNone_ = IsNone_;
        Value_ = NULL;
        IsNone_ = true;
        IsMoved_ = true;
    }

  public:
    typedef T Type;

    Optional() : Value_(NULL), IsNone_(true), IsMoved_(false)
    {
    }

    Optional(None) : Value_(NULL), IsNone_(true), IsMoved_(false)
    {
    }

    Optional(T Value) : Value_(&Value), IsNone_(false), IsMoved_(false)
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
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        return *Value_;
    }

    T Get(T Default) const
    {
        if (IsMoved_)
            return Default;

        if (IsNone_)
            return Default;

        return *Value_;
    }

    operator T() const
    {
        return Get();
    }

    T operator->() const
    {
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        return Value_;
    }

    bool IsSome() const
    {
        return !IsMoved_ && !IsNone_;
    }

    bool IsNone() const
    {
        return !IsMoved_ && IsNone_;
    }

  private:
    typename std::remove_reference<T>::type *Value_;
    bool IsNone_;
};

template <typename T> class Optional<T, typename std::enable_if<!std::is_reference<T>::value>::type>
{
    WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED_EX(Optional, Value_(Other.Value_))

  public:
    Optional Clone() const
    {
        if (IsMoved_)
            throw MovedException();

        Optional clone;
        clone.Value_ = Value_;
        clone.IsNone_ = IsNone_;
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        To.IsMoved_ = IsMoved_;
        if (IsMoved_)
            return;
        To.Value_ = Value_;
        To.IsNone_ = IsNone_;
        Value_ = typename std::remove_const<T>::type();
        IsNone_ = true;
        IsMoved_ = true;
    }

  public:
    typedef T Type;

    Optional() : IsNone_(true), IsMoved_(false)
    {
    }

    Optional(None) : IsNone_(true), IsMoved_(false)
    {
    }

    Optional(T Value) : Value_(Value), IsNone_(false), IsMoved_(false)
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
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        return Value_;
    }

    T Get(const T &Default) const
    {
        if (IsMoved_)
            return Default;

        if (IsNone_)
            return Default;

        return Value_;
    }

    operator T() const
    {
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        return Value_;
    }

    operator typename std::remove_reference<T>::type &()
    {
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        return Value_;
    }

    T operator->() const
    {
        if (IsMoved_)
            throw MovedException();

        if (IsNone_)
            throw Exception();

        return Value_;
    }

    bool IsSome() const
    {
        return !IsMoved_ && !IsNone_;
    }

    bool IsNone() const
    {
        return !IsMoved_ && IsNone_;
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
#else
#endif
} // namespace Win32Ex

#endif // _WIN32EX_OPTIONAL_HPP_
