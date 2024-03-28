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

#define DB2_PACK_SIZE 8
#define DB2_PRAGMA(p) _Pragma(#p)
#define DB2_PRAGMA_PACK(s) DB2_PRAGMA(pack(s))
#define DB2_PRAGMA_PACK_ON DB2_PRAGMA_PACK(DB2_PACK_SIZE) // _Pragma("pack(8)")
#define DB2_PRAGMA_PACK_OFF DB2_PRAGMA_PACK()             // _Pragma("pack()")

#define DB2_NOTE(note)

//
template <typename CK_T, typename = void>
struct has_value_type : std::false_type
{
};
template <typename CK_T>
struct has_value_type<CK_T, std::void_t<typename CK_T::value_type>> : std::true_type
{
};

template <typename T, typename = void>
struct default_value_type
{
    using type = T;
};
template <typename T>
struct default_value_type<T, std::void_t<typename T::value_type>>
{
    using type = typename T::value_type;
};

template <typename T, typename = void>
struct default_ref_type
{
    using type = void;
};
template <typename T>
struct default_ref_type<T, std::enable_if_t<!std::is_void_v<T>>>
{
    using type = T &;
};