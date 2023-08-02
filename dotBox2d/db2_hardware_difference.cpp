#include "db2_hardware_difference.h"

#include <limits> // std::numeric_limits<float>::is_iec559

auto hardwareDifference::getDataStructureAlignment(bool packed) -> unsigned char
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
    ;

    DB2_PRAGMA_PACK_OFF

    if (!packed)
    {
        static const auto offset = (unsigned char)&(((tester *)0)->n);
        return offset;
    }
    else
    {
        static const auto offset_packed = (unsigned char)&(((tester_p *)0)->n);
        return offset_packed;
    }
}

auto hardwareDifference::isLittleEndian() -> bool
{
    // static const unsigned long x = 1;
    // static const bool isLittleEndian = (*((char *)&x) == 1);
    // return isLittleEndian;

    static const bool isLittleEndian = std::endian::native == std::endian::little;
    return isLittleEndian;
}

auto hardwareDifference::isBigEndian() -> bool
{
    static const bool isBigEndian = std::endian::native == std::endian::big;
    return isBigEndian;
}

auto hardwareDifference::reverseEndian(char *source, uint8_t length) -> void
{
    int begin = 0;
    int end = length - 1;
    char temp{};
    while (begin > end)
    {
        temp = source[begin];
        source[begin] = source[end];
        source[end] = temp;
        ++begin;
        --end;
    }
}

auto hardwareDifference::IEEE754() -> bool
{
    static const bool ieee754_f = std::numeric_limits<float>::is_iec559; // IEEE 754
    static const bool digits24_f = (std::numeric_limits<float>::digits == 24);
    static const bool size32_f = (sizeof(float) * 8 == 32);

    static const bool ieee754_d = std::numeric_limits<double>::is_iec559; // IEEE 754
    static const bool digits52_d = (std::numeric_limits<double>::digits == 52);
    static const bool size64_d = (sizeof(double) * 8 == 64);

    return ieee754_f && digits24_f && size32_f && ieee754_d && digits52_d && size64_d;
}

auto hardwareDifference::check() -> bool
{
    return (hardwareDifference::isLittleEndian() || hardwareDifference::isBigEndian()) && (sizeof(bool) == 1 && hardwareDifference::IEEE754());
}