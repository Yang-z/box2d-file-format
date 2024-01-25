#pragma once

#include <cstdint>           // int8_t int16_t int32_t int64_t
using int128_t = __int128_t; // GCC & Clang

// #include <cfloat>
// #include <boost/multiprecision/cpp_bin_float.hpp>
// #include <stdfloat> // c++23
using float32_t = float;
using float64_t = double;

#include <bit> // std::endian (c++20)
#define ENDIAN_SENSITIVE

#define DB2_PACK_SIZE 8
#define DB2_PRAGMA(p) _Pragma(#p)
#define DB2_PRAGMA_PACK(s) DB2_PRAGMA(pack(s))
#define DB2_PRAGMA_PACK_ON DB2_PRAGMA_PACK(DB2_PACK_SIZE) // _Pragma("pack(8)")
#define DB2_PRAGMA_PACK_OFF DB2_PRAGMA_PACK()             // _Pragma("pack()")

#define DB2_NOTE(note)
