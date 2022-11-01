#include "db2_hardware_difference.h"

#include "db2_settings.h"
// #include <arpa/inet.h>

auto hardwareDifference::getDataStructureAlignment(bool packed) -> long long
{
    DB2_PRAGMA_PACK_ON

    struct tester
    {
        const char c;
        const __int128_t n;
    } DB2_NOTE(sizeof(tester));

    struct tester_p
    {
        const char c;
        const __attribute__((packed)) __int128_t n;
    };

    DB2_PRAGMA_PACK_OFF

    if (!packed)
    {
        static const auto offset = (long long)&(((tester *)0)->n);
        return offset;
    }
    else
    {
        static const auto offset_packed = (long long)&(((tester_p *)0)->n);
        return offset_packed;
    }
}

auto hardwareDifference::isLittleEndian() -> bool
{
    static const unsigned long x = 1;
    static const bool isLittleEndian = (*((char *)&x) == 1);
    return isLittleEndian;
}

auto hardwareDifference::reverseEndian(char *source, unsigned long long length) -> void
{
    int start = 0;
    int end = length - 1;
    char temp{};
    while (start > end)
    {
        temp = source[start];
        source[start] = source[end];
        source[end] = temp;
        ++start;
        --end;
    }
}
