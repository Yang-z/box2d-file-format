#include "db2_hardware_difference.h"

auto hardwareDifference::isBigEndian() -> bool
{
    static int x = 1;
    static bool isBigEndian = (((char *)&x) == 0);
    return isBigEndian;
}

auto hardwareDifference::fromDiskToMenory(char* source, char* destination, char length)->void{
    
}

auto hardwareDifference::getMemoryAlignment() -> long long
{
    struct packTester
    {
        const char c;
        const int i;
    };

    static long long offset = (long long)&(((packTester *)0)->i);
    return offset;
}