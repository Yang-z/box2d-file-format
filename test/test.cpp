
#include <iostream>
#include <type_traits> // std::is_same

#include "db2_hardware_difference.h"
#include "db2_data_structure.h"
#include "db2_parser.h"

template <typename T>
auto print(char const *const message, const T &t) -> void
{
    std::cout << message;
    if (std::is_same<T, bool>::value)
    {
        std::cout << (t ? "true" : "false");
    }
    else if (std::is_same<T, uint8_t>::value)
    {
        std::cout << +t;
    }
    else
    {
        std::cout << t;
    }
    std::cout << std::endl;
}

auto test_hardware_difference() -> void
{

    // std::cout << "Is little endian: " << (hardwareDifference::isLittleEndian() ? "true" : "false") << std::endl;
    // std::cout << "Is little endian (bit endianness): " << (hardwareDifference::isLittleEndian_Bit() ? "true" : "false") << std::endl;
    // std::cout << "Date Structure Alignment offset: " << +hardwareDifference::getDataStructureAlignment() << std::endl;
    // std::cout << "Date Structure Alignment offset (packed): " << +hardwareDifference::getDataStructureAlignment(true) << std::endl;

    print("Is little endian: ", hardwareDifference::isLittleEndian());
    print("Is little endian (bit endianness): ", hardwareDifference::isLittleEndian_Bit());
    print("Date Structure Alignment offset: ", hardwareDifference::getDataStructureAlignment());
    print("Date Structure Alignment offset (packed): ", hardwareDifference::getDataStructureAlignment(true));
}

auto test_data_structure_write() -> void
{
    auto db2 = new dotBox2d();
    db2->chunks.info.push();
    db2->chunks.world.push();
    for (int i = 0; i < 2; i++)
        db2->chunks.body.push();
    for (int i = 0; i < 3; i++)
        db2->chunks.fixture.push();
    for (int i = 0; i < 16; i++)
        db2->chunks.vector.push();
    db2->save("./test.B2d");
    delete db2;
    db2 = nullptr;
}

auto test_data_structure_read() -> void
{
    dotBox2d db2{"./test.B2d"};
    return;
}

auto main() -> int
{
    test_hardware_difference();
    // test_data_structure_write();
    test_data_structure_read();

    dotB2Parser p{};

    return 0;
}