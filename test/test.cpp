
#include <iostream>

#include "db2_hardware_difference.h"
#include "db2_data_structure.h"

auto test_hardware_difference() -> void
{
    std::cout << "Is little endian: " << (hardwareDifference::isLittleEndian() ? "true" : "false") << std::endl;
    std::cout << "Date Structure Alignment offset: " << hardwareDifference::getDataStructureAlignment() << std::endl;
    std::cout << "Date Structure Alignment offset (if packed): " << hardwareDifference::getDataStructureAlignment(true) << std::endl;
}

auto test_data_structure() -> void
{
    // auto db2 = new dotBox2d();
    // db2->chunks.info.push();
    // db2->chunks.world.push();
    // for(int i = 0; i<2;i++)
    //     db2->chunks.body.push();
    // for(int i = 0; i<3;i++)
    //     db2->chunks.fixture.push();
    // for(int i = 0; i<16;i++)
    //     db2->chunks.vector.push();
    // db2->save("./test.B2d");
    // delete db2;
    // db2 = nullptr;

    dotBox2d db2{"./test.B2d"};

    return;
}

auto main() -> int
{
    test_hardware_difference();
    test_data_structure();

    return 0;
}