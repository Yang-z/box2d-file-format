#include "db2_hardware_difference.h"

#include <iostream>

int main()
{
    std::cout << "is big endian: " << hardwareDifference::isBigEndian() << std::endl;
    std::cout << "memory alignment offset: " << hardwareDifference::getMemoryAlignment() << std::endl;
}