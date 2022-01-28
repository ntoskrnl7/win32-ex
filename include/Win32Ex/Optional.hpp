/*++

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
        if (isNone_)
            return clone;
        clone.isNone_ = isNone_;
        if (ref_)
            clone.ref_ = new Type(*ref_);
        clone.value_ = NULL;
        return clone;
    }

  protected:
    void Move(OptionalConstStringRef &To)
    {
        To.isMoved_ = isMoved_;
        if (isMoved_)
            return;

        To.value_ = value_;
        To.ref_ = ref_;
        To.isNone_ = isNone_;
        isNone_ = true;
        isMoved_ = true;
    }

    void Move(OptionalString<Type> &To)
    {
        To.isMoved_ = isMoved_;
        if (isMoved_)
            return;

        To.isNone_ = isNone_;
        if (To.isNone_)
            return;

        To.isNull_ = ref_ == NULL;
        if (!To.isNull_)
            To.value_ = *ref_;

        Clear_();
        value_ = NULL;
        ref_ = NULL;
        isNone_ = true;
        isMoved_ = true;
    }

    void Move(OptionalString<const Type> &To)
    {
        To.isMoved_ = isMoved_;
        if (isMoved_)
            return;

        To.isNone_ = isNone_;
        if (To.isNone_)
            return;

        To.isNull_ = ref_ == NULL;
        if (!To.isNull_)
            To.value_ = *ref_;

        Clear_();
        value_ = NULL;
        ref_ = NULL;
        isNone_ = true;
        isMoved_ = true;
    }

  public:
    OptionalConstStringRef() : isNone_(true), ref_(NULL), value_(NULL), isMoved_(false)
    {
    }

    OptionalConstStringRef(None) : isNone_(true), ref_(NULL), value_(NULL), isMoved_(false)
    {
    }

    OptionalConstStringRef(ConstTypeRefType Value) : isNone_(false), ref_(&Value), value_(NULL), isMoved_(false)
    {
    }

    OptionalConstStringRef(ConstPtrType Value) : isNone_(false), ref_(NULL), value_(NULL), isMoved_(false)
    {
        if (Value)
        {
            value_ = Value;
            ref_ = new Type(value_);
        }
    }

    OptionalConstStringRef(const Optional<Type> &Other)
        : isNone_(Other.isNone_), ref_(Other.isNull_ ? NULL : &Other.value_), value_(NULL), isMoved_(Other.isMoved_)
    {
    }

    OptionalConstStringRef(Optional<Type> &Other) : isNone_(false), ref_(NULL), value_(NULL), isMoved_(false)
    {
        Other.Move(*this);
    }

    OptionalConstStringRef(Optional<const Type> &Other) : isNone_(false), ref_(NULL), value_(NULL), isMoved_(false)
    {
        Other.Move(*this);
    }

    ~OptionalConstStringRef()
    {
        Clear_();
    }

    ConstPtrType Get(const Type &Default) const
    {
        if (isMoved_)
            return Default.c_str();

        if (isNone_)
            return Default.c_str();

        return Get();
    }

    ConstPtrType Get() const
    {
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        if (ref_)
            return ref_->c_str();

        return value_;
    }

    operator ConstTypeRefType() const
    {
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        if (!ref_)
            throw NullException();

        return *ref_;
    }

    bool IsSome() const
    {
        return !isMoved_ && !isNone_;
    }

    bool IsNone() const
    {
        return !isMoved_ && isNone_;
    }

    bool IsNull() const
    {
        return !isMoved_ && ref_ == NULL;
    }

  private:
    void Clear_()
    {
        if (value_ && ref_)
            delete ref_;
    }

  private:
    ConstPtrType value_;
    const Type *ref_;
    bool isNone_;
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
        if (isMoved_)
            throw MovedException();

        OptionalString clone;
        clone.value_ = value_;
        clone.isNull_ = isNull_;
        clone.isNone_ = isNone_;
        return clone;
    }

  protected:
    void Move(OptionalString &To)
    {
        To.isMoved_ = isMoved_;
        if (isMoved_)
            return;

        To.value_ = value_;
        To.isNull_ = isNull_;
        To.isNone_ = isNone_;
        To.value_.swap(value_);
        value_.clear();
        isNone_ = true;
        isMoved_ = true;
    }

    void Move(OptionalConstStringRef<_StringType> &To)
    {
        To.isMoved_ = isMoved_;
        if (isMoved_)
            return;

        To.isNone_ = isNone_;
        if (To.isNone_)
            return;

        To.ref_ = (isNull_) ? NULL : new _StringType(value_);
        To.value_ = NULL;
        value_.clear();
        isNone_ = true;
        isMoved_ = true;
    }

  public:
    typedef _StringType Type;
    typedef const Type &ConstRefType;

    typedef typename _StringType::value_type CharType;
    typedef const CharType *ConstPtrType;

    OptionalString() : isNone_(true), isMoved_(false)
    {
    }

    OptionalString(None) : isNone_(true), isMoved_(false)
    {
    }

    OptionalString(Optional<const typename std::remove_const<Type>::type &> &Other) : isNone_(false), isMoved_(false)
    {
        Other.Move(*this);
    }

    OptionalString(const Optional<const typename std::remove_const<Type>::type &> &Other)
        : isNone_(Other.IsNone()), isMoved_(Other.IsMoved())
    {
        if (isNone_)
            return;

        isNull_ = Other.IsNull();
        if (!isNull_)
            value_ = Other;
    }

    OptionalString(ConstPtrType Value) : isNone_(false), isMoved_(false)
    {
        isNull_ = Value == NULL;
        if (!isNull_)
            value_ = Value;
    }

    OptionalString(ConstRefType Value) : isNone_(false), value_(Value), isNull_(false), isMoved_(false)
    {
    }

    operator ConstRefType() const
    {
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        if (isNull_)
            throw NullException();

        return value_;
    }

    operator Type &()
    {
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        if (isNull_)
            throw NullException();

        return value_;
    }

    ConstPtrType Get(ConstRefType Default) const
    {
        if (isMoved_)
            throw Default.c_str();

        if (isNone_)
            return Default.c_str();

        return Get();
    }

    ConstPtrType Get() const
    {
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        return isNull_ ? NULL : value_.c_str();
    }

    bool IsSome() const
    {
        return !isMoved_ && !isNone_;
    }

    bool IsNone() const
    {
        return !isMoved_ && isNone_;
    }

    bool IsNull() const
    {
        return !isMoved_ && isNull_;
    }

  protected:
    typename std::remove_const<Type>::type value_;
    bool isNull_;
    bool isNone_;
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
        return OptionalConstStringRef::isMoved_;
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
        return OptionalConstStringRef::isMoved_;
    }

    Optional Clone() const
    {
        if (OptionalConstStringRef<Type>::isMoved_)
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
        return OptionalString::isMoved_;
    }

    Optional Clone() const
    {
        if (OptionalString<Type>::isMoved_)
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
        return OptionalString::isMoved_;
    }

    Optional Clone() const
    {
        if (OptionalString<Type>::isMoved_)
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
        return OptionalString::isMoved_;
    }

    Optional Clone() const
    {
        if (OptionalString<Type>::isMoved_)
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
        return OptionalString::isMoved_;
    }

    Optional Clone() const
    {
        if (OptionalString<Type>::isMoved_)
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
    WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED_EX(Optional, value_(Other.value_))

  public:
    Optional Clone() const
    {
        if (isMoved_)
            throw MovedException();

        Optional clone;
        clone.value_ = value_;
        clone.isNone_ = isNone_;
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        To.isMoved_ = isMoved_;
        if (isMoved_)
            return;

        To.value_ = value_;
        To.isNone_ = isNone_;
        value_ = NULL;
        isNone_ = true;
        isMoved_ = true;
    }

  public:
    typedef T Type;

    Optional() : value_(NULL), isNone_(true), isMoved_(false)
    {
    }

    Optional(None) : value_(NULL), isNone_(true), isMoved_(false)
    {
    }

    Optional(T Value) : value_(&Value), isNone_(false), isMoved_(false)
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
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        return *value_;
    }

    T Get(T Default) const
    {
        if (isMoved_)
            return Default;

        if (isNone_)
            return Default;

        return *value_;
    }

    operator T() const
    {
        return Get();
    }

    T operator->() const
    {
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        return value_;
    }

    bool IsSome() const
    {
        return !isMoved_ && !isNone_;
    }

    bool IsNone() const
    {
        return !isMoved_ && isNone_;
    }

  private:
    typename std::remove_reference<T>::type *value_;
    bool isNone_;
};

template <typename T> class Optional<T, typename std::enable_if<!std::is_reference<T>::value>::type>
{
    WIN32EX_MOVE_ALWAYS_CLASS_WITH_IS_MOVED_EX(Optional, value_(Other.value_))

  public:
    Optional Clone() const
    {
        if (isMoved_)
            throw MovedException();

        Optional clone;
        clone.value_ = value_;
        clone.isNone_ = isNone_;
        return clone;
    }

  private:
    void Move(Optional &To)
    {
        To.isMoved_ = isMoved_;
        if (isMoved_)
            return;
        To.value_ = value_;
        To.isNone_ = isNone_;
        value_ = typename std::remove_const<T>::type();
        isNone_ = true;
        isMoved_ = true;
    }

  public:
    typedef T Type;

    Optional() : isNone_(true), isMoved_(false)
    {
    }

    Optional(None) : isNone_(true), isMoved_(false)
    {
    }

    Optional(T Value) : value_(Value), isNone_(false), isMoved_(false)
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
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        return value_;
    }

    T Get(const T &Default) const
    {
        if (isMoved_)
            return Default;

        if (isNone_)
            return Default;

        return value_;
    }

    operator T() const
    {
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        return value_;
    }

    operator T &()
    {
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        return value_;
    }

    T operator->() const
    {
        if (isMoved_)
            throw MovedException();

        if (isNone_)
            throw Exception();

        return value_;
    }

    bool IsSome() const
    {
        return !isMoved_ && !isNone_;
    }

    bool IsNone() const
    {
        return !isMoved_ && isNone_;
    }

  private:
    typename std::remove_const<T>::type value_;
    bool isNone_;
};

#if defined(__cpp_variadic_templates)
template <typename... Args> bool IsAll(const Optional<Args> &... args)
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

template <typename... Args> bool IsSome(const Optional<Args> &... args)
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

template <typename... Args> bool IsAny(const Optional<Args> &... args)
{
    return IsSome(args...);
}

template <typename... Args> bool IsNone(const Optional<Args> &... args)
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
template <typename Arg0, typename Arg1> bool IsAll(const Optional<Arg0> &arg0, const Optional<Arg1> &arg1)
{
    return arg0.IsSome() && arg1.IsSome();
}

template <typename Arg0, typename Arg1> bool IsSome(const Optional<Arg0> &arg0, const Optional<Arg1> &arg1)
{
    return arg0.IsSome() || arg1.IsSome();
}

template <typename Arg0, typename Arg1> bool IsAny(const Optional<Arg0> &arg0, const Optional<Arg1> &arg1)
{
    return IsSome(arg0, arg1);
}

template <typename Arg0, typename Arg1> bool IsNone(const Optional<Arg0> &arg0, const Optional<Arg1> &arg1)
{
    return arg0.IsNone() && arg1.IsNone();
}

template <typename Arg0, typename Arg1, typename Arg2>
bool IsAll(const Optional<Arg0> &arg0, const Optional<Arg1> &arg1, const Optional<Arg2> &arg2)
{
    return arg0.IsSome() && arg1.IsSome() && arg2.IsSome();
}

template <typename Arg0, typename Arg1, typename Arg2>
bool IsSome(const Optional<Arg0> &arg0, const Optional<Arg1> &arg1, const Optional<Arg2> &arg2)
{
    return arg0.IsSome() || arg1.IsSome() || arg2.IsSome();
}

template <typename Arg0, typename Arg1, typename Arg2>
bool IsAny(const Optional<Arg0> &arg0, const Optional<Arg1> &arg1, const Optional<Arg2> &arg2)
{
    return IsSome(arg0, arg1, arg2);
}

template <typename Arg0, typename Arg1, typename Arg2>
bool IsNone(const Optional<Arg0> &arg0, const Optional<Arg1> &arg1, const Optional<Arg2> &arg2)
{
    return arg0.IsNone() && arg1.IsNone() && arg2.IsNone();
}
#endif // defined(__cpp_variadic_templates)
} // namespace Win32Ex

#endif // _WIN32EX_OPTIONAL_HPP_
