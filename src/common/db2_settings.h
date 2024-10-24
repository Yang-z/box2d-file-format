#pragma once

#include <cstdint>           // int8_t int16_t int32_t int64_t
using int128_t = __int128_t; // GCC & Clang

// #include <cfloat>
// #include <boost/multiprecision/cpp_bin_float.hpp>
// #include <stdfloat> // c++23
using float32_t = float;
using float64_t = double;

#define ENDIAN_SENSITIVE
#define TYPE_IRRELATIVE /* type-irrelative */

#define DB2_PACK_SIZE 8 // byte
#define DB2_PRAGMA(p) _Pragma(#p)
#define DB2_PRAGMA_PACK(s) DB2_PRAGMA(pack(s))
#define DB2_PRAGMA_PACK_ON DB2_PRAGMA_PACK(DB2_PACK_SIZE) // _Pragma("pack(8)")
#define DB2_PRAGMA_PACK_OFF DB2_PRAGMA_PACK()             // _Pragma("pack()")

#define DB2_NOTE(note)
#define DB2_SEMICOLON ;
#define DB2_ASSERT(assert) DB2_SEMICOLON static_assert(assert, #assert)

//
#include <type_traits> // std::void_t

#define HAS_TYPE(FLAG_T)                                                           \
    template <typename CK_T, typename = void>                                      \
    struct has_##FLAG_T : std::false_type                                          \
    {                                                                              \
    };                                                                             \
    template <typename CK_T>                                                       \
    struct has_##FLAG_T<CK_T, std::void_t<typename CK_T::FLAG_T>> : std::true_type \
    {                                                                              \
    };                                                                             \
    template <typename CK_T>                                                       \
    inline constexpr bool has_##FLAG_T##_v = has_##FLAG_T<CK_T>::value;

HAS_TYPE(value_type) // has_value_type

// template <typename CK_T, typename = void>
// struct has_value_type : std::false_type
// {
// };
// template <typename CK_T>
// struct has_value_type<CK_T, std::void_t<typename CK_T::value_type>> : std::true_type
// {
// };
// template <typename CK_T>
// inline constexpr bool has_value_type_v = has_value_type<CK_T>::value;

template <typename T, typename = void>
struct default_value
{
    using type = T;
};
template <typename T>
struct default_value<T, std::void_t<typename T::value_type>>
{
    using type = typename T::value_type;
};
template <typename T>
using default_value_t = typename default_value<T>::type;

template <typename T, typename = void>
struct default_ref
{
    using type = void;
};
template <typename T>
struct default_ref<T, std::enable_if_t<!std::is_void_v<T>>>
{
    using type = T &;
};
template <typename T>
using default_ref_t = typename default_ref<T>::type;