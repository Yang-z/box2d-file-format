
#include <iostream>
#include <type_traits> // std::is_same

#include "db2_hardware_difference.h"
#include "db2_data_structure.h"
#include "db2_decoder.h"

#include <boost/pfr.hpp> // reflect
// #include <boost/pfr/core.hpp>
// #include <boost/pfr/core_name.hpp>

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
}

auto test_reflection() -> void
{
    struct S0
    {
        int i0{8};
        bool b0{true};

        struct
        {
            int i1{4};
            float b2{2.0f};
        } s1;
    } s0;

    int count = 0;
    boost::pfr::for_each_field(
        s0,
        [&s0, &count](auto &chunk)
        {
            ++count;
            printf("%d\n", sizeof(chunk));
            // printf("%d\n", boost::pfr::get_name<count, s0>());
        });
    printf("count: %d\n", count);
}

auto test_data_structure_write() -> void
{
    auto db2 = new dotBox2d();
    db2->chunks.info.emplace_back();
    db2->chunks.world.emplace_back();
    for (int i = 0; i < 2; i++)
        db2->chunks.body.emplace_back();
    for (int i = 0; i < 3; i++)
        db2->chunks.fixture.emplace_back();
    for (int i = 0; i < 16; i++)
        db2->chunks.vector.emplace_back();
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
    der.db2->save("./test_encode.B2d");

    test_step(der.b2w);
}

auto test_decoding() -> void
{
    dotB2Decoder der{};
    der.db2 = new dotBox2d{"./test_encode.B2d"};
    der.decode();

    test_step(der.b2w);
}

auto main() -> int
{
    test_hardware_difference();

    // test_reflection();

    // test_data_structure_write();
    // test_data_structure_read();

    test_encoding();
    test_decoding();

    return 0;
}