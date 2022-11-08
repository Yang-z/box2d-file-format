#include "db2_hardware_difference.h"
#include "db2_data_structure.h"

#include <iostream>

auto test_hardware_difference() -> void
{
    std::cout << "Is little endian: " << (hardwareDifference::isLittleEndian() ? "true" : "false") << std::endl;
    std::cout << "Date Structure Alignment offset: " << hardwareDifference::getDataStructureAlignment() << std::endl;
    std::cout << "Date Structure Alignment offset (packed): " << hardwareDifference::getDataStructureAlignment(true) << std::endl;
}

auto test_data_structure() -> void
{
    auto db2 = new dotBox2d();

    // db2->info.count.world=1;
    // db2->info.count.body=2;
    // db2->info.count.fixture=3;
    // db2->info.count.vec2=8;

    // db2->world=new dotB2Wrold[1];
    // db2->body = new dotB2Body[2];
    // db2->fixture=new dotB2Fixture[3];
    // db2->vec2=new dotB2Vec2[8];

    // db2->save("./test.B2d");

    db2->load("./test.B2d");

    delete db2;
    db2 = nullptr;

    return;
}

auto main() -> int
{
    test_hardware_difference();
    test_data_structure();

    return 0;
}