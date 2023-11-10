
#include <iostream>
#include <type_traits> // std::is_same

#include "db2_hardware_difference.h"
#include "db2_data_structure.h"
#include "db2_decoder.h"

#include <boost/pfr.hpp> // reflect
// #include <boost/pfr/core.hpp>
// #include <boost/pfr/core_name.hpp>

auto test_pointer_cast() -> void
{
    int *data{nullptr};
    printf("data = %p\n", data); // 0000000000000000
    *(void **)(&data) = malloc(8);
    printf("data = %p\n", data); // 00000245044613a0
    // printf("*(void **)(&data) + 1 = %p\n", *(void **)(&data) + 1); // 00000245044613a1
    printf("*(char **)(&data) + 1 = %p\n", *(char **)(&data) + 1); // 00000245044613a1
    printf("*(int  **)(&data) + 1 = %p\n", *(int **)(&data) + 1);  // 00000245044613a4

    char type[4];
    printf("type = %p\n", type);         // 00000016aafffab4
    printf("type + 1 = %p\n", type + 1); // 00000016aafffab5
}

auto test_equality() -> void
{
    char sc = 0b11111111;
    unsigned char uc = 0b11111111;

    printf("char c = 0b11111111;\nunsigned char uc = 0b11111111;\n");
    printf("sc == uc; //%s\n", sc == uc ? "true" : "false");                                   // false
    printf("std::equal(&sc, &sc, &uc); //%s\n", std::equal(&sc, &sc, &uc) ? "true" : "false"); // true
    printf("\n");

    int8_t si8 = -1;
    uint8_t ui8 = -1;
    printf("signed char si8 = -1; //%d\nunsigned char ui8 = -1; //%d\n", si8, ui8);
    printf("si8 == ui8; //%s\n", si8 == ui8 ? "true" : "false");                                     // false
    printf("std::equal(&si8, &si8, &ui8); //%s\n", std::equal(&si8, &si8, &ui8) ? "true" : "false"); // true
    printf("\n");
    /*
    For the comparisons between signed char and unsigned char, si8 is assigned the value -1, while
    ui8 is also assigned the value -1. However, they have different types, with si8 being a signed
    char and ui8 being an unsigned char.
    When comparing them, the compiler converts them both to a common type for comparison, which
    in this case is int:
        signed      0b11111111 -> 0b11111111 11111111 11111111 11111111
        unsigned    0b11111111 -> 0b00000000 00000000 00000000 11111111
    */

    int16_t si16 = -1;
    uint16_t ui16 = -1;
    printf("signed char si16 = -1;\nunsigned char ui16 = -1;\n");
    printf("si16 == ui16; //%s\n", si16 == ui16 ? "true" : "false");                                       // false
    printf("std::equal(&si16, &si16, &ui16); //%s\n", std::equal(&si16, &si16, &ui16) ? "true" : "false"); // true
    printf("\n");

    int32_t si32 = -1;
    uint32_t ui32 = -1;
    printf("signed int si32 = -1; //%d\nunsigned int ui32 = -1; //%d\n", si32, ui32);
    printf("si32 == ui32; //%s\n", si32 == ui32 ? "true" : "false");                                       // true?!
    printf("std::equal(&si32, &si32, &ui32); //%s\n", std::equal(&si32, &si32, &ui32) ? "true" : "false"); // true
    printf("\n");

    int64_t si64 = -1;
    uint64_t ui64 = -1;
    printf("int64_t si64 = -1;\nuint64_t ui64 = -1;\n");
    printf("si64 == ui64; //%s\n", si64 == ui64 ? "true" : "false");                                       // true?!
    printf("std::equal(&si64, &si64, &ui64); //%s\n", std::equal(&si64, &si64, &ui64) ? "true" : "false"); // true
    printf("\n");

    printf("-1 == UINT8_MAX //%s\n", -1 == UINT8_MAX ? "true" : "false");   // false
    printf("-1 == UINT16_MAX //%s\n", -1 == UINT16_MAX ? "true" : "false"); // false
    printf("-1 == UINT32_MAX //%s\n", -1 == UINT32_MAX ? "true" : "false"); // true
    printf("-1 == UINT64_MAX //%s\n", -1 == UINT64_MAX ? "true" : "false"); // true
    printf("\n");
}

auto test_hardware_difference() -> void
{

    // std::cout << "Is little endian: " << (hardwareDifference::isLittleEndian() ? "true" : "false") << std::endl;
    // std::cout << "Is little endian (bit endianness): " << (hardwareDifference::isLittleEndian_Bit() ? "true" : "false") << std::endl;
    // std::cout << "Date Structure Alignment offset: " << +hardwareDifference::getDataStructureAlignment() << std::endl;
    // std::cout << "Date Structure Alignment offset (packed): " << +hardwareDifference::getDataStructureAlignment(true) << std::endl;

    printf("Is little endian: %s\n", hardwareDifference::isLittleEndian() ? "true" : "false");
    printf("Is little endian (bit endianness): %s\n", hardwareDifference::isLittleEndian_Bit() ? "true" : "false");
    printf("Date Structure Alignment offset: %d\n", hardwareDifference::getDataStructureAlignment());
    printf("Date Structure Alignment offset (packed): %d\n", hardwareDifference::getDataStructureAlignment(true));

    int32_t i4r = 8;
    hardwareDifference::reverseEndian((char *)&i4r, sizeof(i4r));
    printf("int32_t i32 = 8; //reversed = %d\n", i4r);
}

auto test_reflection() -> void
{
    struct S
    {
        int i0{22};
        bool b0{true};
        // int ia[3]{1, 2, 3}; // array is not supported
        struct
        {
            int i1{33};
            float b2{44.0f};
        } s1;
    } s0;

    float s1 = 3.14;

    int count = 0;
    boost::pfr::for_each_field(
        s0,
        [&s0, &count](auto &chunk)
        {
            ++count;
            printf("%d: %d\n", sizeof(chunk), chunk);
            // printf("%d\n", boost::pfr::get_name<count, s0>());
        });
    printf("count: %d\n", count);

    auto size = boost::pfr::tuple_size<S>::value;
    for (std::size_t i = 0; i < size; ++i)
    {
        // auto value = boost::pfr::get<i>(s0); // no
    }
}

auto test_CRC() -> void
{
    auto data = "test";

    auto chunk = db2Chunk<char>{};
    auto crc = chunk.calculateCRC(data, 4);

    printf("%X\n", crc);
}

auto test_data_structure_write() -> void
{
    auto db2 = new dotBox2d();
    db2->chunk<dotB2Info>().emplace_back();
    db2->chunk<dotB2Wrold>().emplace_back();
    for (int i = 0; i < 2; i++)
        db2->chunk<dotB2Body>().emplace_back();
    for (int i = 0; i < 3; i++)
        db2->chunk<dotB2Fixture>().emplace_back();
    for (int i = 0; i < 16; i++)
        db2->chunk<float32_t>().emplace_back();
    db2->save("./test.B2d");
    delete db2;
    db2 = nullptr;
}

auto test_data_structure_read() -> void
{
    dotBox2d db2{"./test.B2d"};
    return;
}

auto test_step(b2World *b2w) -> void
{
    auto dynamicBody = b2w->GetBodyList()->GetNext();

    int t = 0;
    while (true)
    {
        b2w->Step(1.0f / 60.0f, 6, 2);
        printf("Py = %f; Vy = %f\n", dynamicBody->GetPosition().y, dynamicBody->GetLinearVelocity().y);
        if (t > 200)
            break;
        t++;
    }
    printf("step count: %d\n", t);
}

auto test_encoding() -> void
{
    dotB2Decoder der{};
    der.b2w = new b2World{{0.0f, -9.8f}};

    /*body*/
    b2BodyDef dynamicBodyDef{};
    dynamicBodyDef.type = b2_dynamicBody;
    dynamicBodyDef.position.Set(8.0f, 8.0f);

    auto dynamicBody = der.b2w->CreateBody(&dynamicBodyDef);

    /*fixture*/
    b2CircleShape circle{};
    circle.m_radius = 0.5f;

    b2FixtureDef fixturedef{};
    fixturedef.shape = &circle;
    fixturedef.density = 1.0f;
    fixturedef.restitution = 0.8f;

    dynamicBody->CreateFixture(&fixturedef);

    /*body2*/
    b2BodyDef staticBodyDef{};
    staticBodyDef.type = b2_staticBody;
    staticBodyDef.position.Set(0.0f, 0.0f);

    auto staticBody = der.b2w->CreateBody(&staticBodyDef);

    /*fixture2*/
    b2ChainShape chain{};
    // b2Vec2 vs[4]{{0.0f, 0.0f}, {16.0f, 0.0f}, {16.0f, 9.0f}, {0.0f, 9.0f}}; // anti-clockw, ouside
    b2Vec2 vs[4]{{0.0f, 0.0f}, {0.0f, 9.0f}, {16.0f, 9.0f}, {16.0f, 0.0f}}; // inside
    chain.CreateLoop(vs, 4);

    b2FixtureDef fixturedef2{};
    fixturedef2.shape = &chain;

    staticBody->CreateFixture(&fixturedef2);

    der.encode();
    der.db2->save("./test_encode.B2d", true);
    der.db2->save("./test_encode_BE.B2D", false);

    test_step(der.b2w);
}

auto test_decoding() -> void
{
    dotB2Decoder der{};
    // der.db2 = new dotBox2d{"./test_encode.B2d"};
    der.db2 = new dotBox2d{"./test_encode_BE.B2d"};
    der.decode();

    test_step(der.b2w);
}

auto main() -> int
{
    // test_pointer_cast();

    // test_equality();

    // test_hardware_difference();

    // test_reflection();

    // test_CRC();

    // test_data_structure_write();
    // test_data_structure_read();

    test_encoding();
    test_decoding();

    return 0;
}