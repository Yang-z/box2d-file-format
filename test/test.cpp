#include "db2_hardware_difference.h"
#include "db2_data_structure.h"

#include <iostream>

auto test_hardware_difference() -> void
{
    std::cout << "is little endian: " << hardwareDifference::isLittleEndian() << std::endl;
    std::cout << "Date Structure Alignment offset: " << hardwareDifference::getDataStructureAlignment() << std::endl;
    std::cout << "Date Structure Alignment offset (packed): " << hardwareDifference::getDataStructureAlignment(true) << std::endl;
}

auto test_data_structure() -> void
{
    auto db2 = new dotBox2d();
    db2->save("./test.b2");
    // db2->load("./test.b2");

    return;
}

auto main() -> int
{
    test_hardware_difference();
    test_data_structure();

    return 0;
}