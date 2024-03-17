#include "db2_hardware_difference.h"

#include <cassert>
#include <limits> // std::numeric_limits<float>::is_iec559

bool hardwareDifference::IsCompatible = hardwareDifference::Check();

auto hardwareDifference::GetDataStructureAlignment(const bool packed) -> uint8_t
{
    DB2_PRAGMA_PACK_ON

    struct tester
    {
        const char c;
        const int128_t n;
    } DB2_NOTE(sizeof(tester));

    struct tester_p
    {
        const char c;
        const __attribute__((packed)) int128_t n;
    } DB2_NOTE(sizeof(tester_p));

    DB2_PRAGMA_PACK_OFF

    if (!packed)
    {
        static const uint8_t offset = (uintptr_t) & (((tester *)0)->n);
        return offset;
    }
    else
    {
        static const uint8_t offset_packed = (uintptr_t) & (((tester_p *)0)->n);
        return offset_packed;
    }
}

auto hardwareDifference::IsLittleEndian() -> bool
{
    // static const unsigned long x = 1;
    // static const bool isLittleEndian = (*((char *)&x) == 1);
    // return isLittleEndian;

    static const bool isLittleEndian = std::endian::native == std::endian::little;
    return isLittleEndian;
}

auto hardwareDifference::IsBigEndian() -> bool
{
    static const bool isBigEndian = std::endian::native == std::endian::big;
    return isBigEndian;
}

auto hardwareDifference::ReverseEndian(char *source, const uint8_t length) -> void
{
    int begin = 0;
    int end = length - 1;
    char temp{};
    while (begin < end)
    {
        temp = source[begin];
        source[begin] = source[end];
        source[end] = temp;
        ++begin;
        --end;
    }
}

auto hardwareDifference::IsLittleEndian_Bit() -> bool
{
    struct bit_order
    {
        bool a : 1;
        uint8_t b : 2;
        uint8_t c : 3;
        uint8_t d : 2;
    } DB2_NOTE(sizeof(bit_order));

    static const uint8_t ch = 0b01001001;
    static struct bit_order const *const ptr = (struct bit_order *)&ch;

    /*
    "bit address"    0    1    2    3    4    5    6    7

    big endian       0    1    0    0    1    0    0    1
    little endian    1    0    0    1    0    0    1    0
                     ~    ~~~~~~    ~~~~~~~~~~~    ~~~~~~
                     a       b           c           d
    big endian     false  0b10(2)     0b010(2)     0b01(1)
    little endian  true   0b00(0)     0b001(1)     0b01(1)
    */

    return ptr->a;
}

auto hardwareDifference::IEEE754() -> bool
{
    static const bool ieee754_f = std::numeric_limits<float>::is_iec559; // IEEE 754
    static const bool digits24_f = (std::numeric_limits<float>::digits == 24);
    static const bool size32_f = (sizeof(float) * 8 == 32);

    static const bool ieee754_d = std::numeric_limits<double>::is_iec559; // IEEE 754
    static const bool digits53_d = (std::numeric_limits<double>::digits == 53);
    static const bool size64_d = (sizeof(double) * 8 == 64);

    static const bool result = ieee754_f && digits24_f && size32_f && ieee754_d && digits53_d && size64_d;

    return result;
}

auto hardwareDifference::Check() -> bool
{
    static const bool result =
        nullptr == 0 &&
        (sizeof(bool) == 1) && (sizeof(char) == 1) &&
        (hardwareDifference::IsLittleEndian() || hardwareDifference::IsBigEndian()) &&
        hardwareDifference::IEEE754();
    assert(result);

    return result;
}