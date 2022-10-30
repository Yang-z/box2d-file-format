#include "db2_hardware_difference.h"

#include "db2_settings.h"

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
        static long long offset = (long long)&(((tester *)0)->n);
        return offset;
    }
    else
    {
        /*
        // packed ignored
        static const auto __attribute__((packed)) p = tester{};
        static long long offset_packed = (long long)&(p.n) - (long long)&(p.c);
        */

        static long long offset_packed = (long long)&(((tester_p *)0)->n);
        return offset_packed;
    }
}

auto hardwareDifference::isBigEndian() -> bool
{
    static long long x = 1;
    static bool isBigEndian = (((char *)&x) == 0);
    return isBigEndian;
}

auto hardwareDifference::reverseEndian(char *source, int length) -> void
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
