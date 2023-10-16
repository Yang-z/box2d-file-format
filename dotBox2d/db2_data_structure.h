#pragma once

#include "db2_hardware_difference.h"
#include "db2_container.h"

DB2_PRAGMA_PACK_ON

ENDIAN_SENSITIVE struct dotB2Fixture
{
    float32_t friction{0.2f};
    float32_t restitution{0.0f};
    float32_t restitutionThreshold{1.0f};
    float32_t density{0.0f};
    bool isSensor{false};

    /* 1 byte gape */

    uint16_t filter_categoryBits{0x0001};
    uint16_t filter_maskBits{0xFFFF};
    int16_t filter_groupIndex{0};

    int32_t shape_type{-0};
    float32_t shape_radius{-0.0f};
    // bool shape_oneSided;//?

    int32_t shape_vecList{-0};
    int32_t shape_vecCount{0};

    uint64_t userData{-0};
} DB2_NOTE(sizeof(dotB2Fixture) == 48);

ENDIAN_SENSITIVE struct dotB2Body
{
    int32_t type{0};
    float32_t position_x{0.0f};
    float32_t position_y{0.0f};
    float32_t angle{0.0f};
    float32_t linearVelocity_x{0.0f};
    float32_t linearVelocity_y{0.0f};
    float32_t angularVelocity{0.0f};
    float32_t linearDamping{0.0f};
    float32_t angularDamping{0.0f};
    bool allowSleep{true};
    bool awake{true};
    bool fixedRotation{false};
    bool bullet{false};
    bool enabled{true};
    /* 3 bytes gape*/
    float32_t gravityScale{1.0f};

    int32_t fixtureList{-0};
    int32_t fixtureCount{0};

    uint64_t userData{-0};
} DB2_NOTE(sizeof(dotB2Body) == 64);

ENDIAN_SENSITIVE struct dotB2Joint
{
    int32_t type{0};
    int32_t bodyA{-0}; // index
    int32_t bodyB{-0}; // index
    bool collideConnected{false};
    /* 3 bytes gape*/
    int32_t para; // index, and for a specified type of joint, the length is fixed.
    /* 4 bytes gape*/
    uint64_t userData{-0};

} DB2_NOTE(sizeof(dotB2Joint) == 32);

ENDIAN_SENSITIVE struct dotB2Wrold
{
    float32_t gravity_x{0.0f};
    float32_t gravity_y{0.0f};

    int32_t bodyList{-0};
    int32_t bodyCount{0};

    int32_t jointList{-0};
    int32_t jointCount{0};
} DB2_NOTE(sizeof(dotB2Wrold) == 24);

struct dotB2Info
{
    const uint8_t packSize{DB2_PACK_SIZE};
    bool isLittleEndian{hardwareDifference::isLittleEndian()};

    struct
    {
        const uint8_t dotBox2d[3]{0, 0, 1};
        const uint8_t box2d[3]{2, 4, 1};
    } version DB2_NOTE(sizeof(version) == 6);

    // ENDIAN_SENSITIVE struct Count
    // {
    //     int32_t world{0};
    //     int32_t body{0};
    //     int32_t fixture{0};
    //     int32_t joint{0};
    //     int32_t vector{0};
    // } count DB2_NOTE(sizeof(count));

} DB2_NOTE(sizeof(dotB2Info) == 8);

DB2_PRAGMA_PACK_OFF

class dotBox2d
{
public:
    // struct
    // {
    //     const char INFO[4]{'I', 'N', 'F', 'O'};
    //     const char WRLD[4]{'W', 'R', 'L', 'D'};
    //     const char JOIN[4]{'J', 'O', 'I', 'N'};
    //     const char BODY[4]{'B', 'O', 'D', 'Y'};
    //     const char FXTR[4]{'F', 'X', 'T', 'R'};
    //     const char VECT[4]{'V', 'E', 'C', 'T'};
    // } DB2_NOTE(sizeof(ChunkTypes)) const chunkTypes{};

    // const dotBox2d::ChunkTypes chunkTypes{};

    uint8_t head[8]{0xB2, 0x42, 0x32, 0x64, 0x0D, 0x0A, 0x1A, 0x0A};

    struct
    {
        db2Container<dotB2Info> info{"INFO"};
        db2Container<dotB2Wrold> world{"WRLD"};
        db2Container<dotB2Joint> joint{"JOIN"};
        db2Container<dotB2Body> body{"BODY"};
        db2Container<dotB2Fixture> fixture{"FXTR"};
        db2Container<float32_t> vector{"VECT"};
        /* user data */
    } chunks;

    dotBox2d(const char *file = nullptr);

public:
    auto load(const char *filePath) -> void;
    auto save(const char *filePath) -> void;
    auto reverseEndian() -> void;
};
