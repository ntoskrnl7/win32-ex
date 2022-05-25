#pragma once

#define WIN32EX_API_DEFINE_STRUCT_T(_TYPE_NAME_)                                                                       \
    WIN32EX_API_DEFINE_STRUCT_T_EX(_TYPE_NAME_, _TYPE_NAME_##T, _TYPE_NAME_##A, _TYPE_NAME_##W)

#define WIN32EX_API_DEFINE_STRUCT_T_EX(_TYPE_NAME_, _TYPE_T_NAME_, _TYPE_A_NAME_, _TYPE_W_NAME_)                       \
    template <typename CharType = TCHAR> struct _TYPE_T_NAME_ : _TYPE_NAME_                                           \
    {                                                                                                                  \
        typedef _TYPE_NAME_ Type;                                                                                      \
    };                                                                                                                 \
    template <> struct _TYPE_T_NAME_<CHAR> : _TYPE_A_NAME_                                                             \
    {                                                                                                                  \
        typedef _TYPE_A_NAME_ Type;                                                                                    \
    };                                                                                                                 \
    template <> struct _TYPE_T_NAME_<WCHAR> : _TYPE_W_NAME_                                                            \
    {                                                                                                                  \
        typedef _TYPE_W_NAME_ Type;                                                                                    \
    };

#define WIN32EX_API_DEFINE_FN_T(_TYPE_NAME_) WIN32EX_API_DEFINE_FN_T_EX(_TYPE_NAME_##T, _TYPE_NAME_##A, _TYPE_NAME_##W)

#define WIN32EX_API_DEFINE_FN_T_EX(_TYPE_T_NAME_, _TYPE_A_NAME_, _TYPE_W_NAME_)                                        \
    template <typename CharType> struct _TYPE_T_NAME_                                                                 \
    {                                                                                                                  \
    };                                                                                                                 \
    template <> struct _TYPE_T_NAME_<CHAR>                                                                             \
    {                                                                                                                  \
        typedef _TYPE_A_NAME_ Type;                                                                                    \
    };                                                                                                                 \
    template <> struct _TYPE_T_NAME_<WCHAR>                                                                            \
    {                                                                                                                  \
        typedef _TYPE_W_NAME_ Type;                                                                                    \
    };
