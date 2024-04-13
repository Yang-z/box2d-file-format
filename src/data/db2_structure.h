#pragma once

#include "containers/db2_chunk.h"

DB2_PRAGMA_PACK_ON

ENDIAN_SENSITIVE struct db2Shape : public db2ChunkStruct<float32_t>
{
};

ENDIAN_SENSITIVE struct db2Fixture
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
} DB2_ASSERT(sizeof(db2Fixture) == 24);

ENDIAN_SENSITIVE struct db2Body
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

} DB2_ASSERT(sizeof(db2Body) == 48);

ENDIAN_SENSITIVE struct db2Joint : public db2ChunkStruct<float32_t>
{
};

ENDIAN_SENSITIVE struct db2World
{
    float32_t gravity_x{0.0f};
    float32_t gravity_y{0.0f};

} DB2_ASSERT(sizeof(db2World) == 8);

struct db2Info
{
    const uint8_t packSize{DB2_PACK_SIZE};
    const uint8_t notUsed{0}; // bool isLittleEndian{HardwareDifference::IsLittleEndian()};

    uint8_t ver_dotBox2d_0{0};
    uint8_t ver_dotBox2d_1{0};
    uint8_t ver_dotBox2d_2{1};

    uint8_t ver_box2d_0{2};
    uint8_t ver_box2d_1{4};
    uint8_t ver_box2d_2{1};

} DB2_ASSERT(sizeof(db2Info) == 8);

DB2_PRAGMA_PACK_OFF

using CKInfo = db2Chunk<db2Info>;
using CKWorld = db2Chunk<db2World>;
using CKJoint = db2Chunk<db2Joint>;
using CKBody = db2Chunk<db2Body>;
using CKFixture = db2Chunk<db2Fixture>;
using CKShape = db2Chunk<db2Shape>;

struct db2ChunkType
{
    static constexpr const char INFO[4]{'I', 'N', 'F', 'O'};
    static constexpr const char WRLD[4]{'W', 'R', 'L', 'D'};
    static constexpr const char JInT[4]{'J', 'I', 'n', 'T'};
    static constexpr const char BODY[4]{'B', 'O', 'D', 'Y'};
    static constexpr const char FXTR[4]{'F', 'X', 'T', 'R'};
    static constexpr const char SHpE[4]{'S', 'H', 'p', 'E'};

    static bool IsRegistered;
    static bool RegisterType();

} DB2_NOTE(sizeof(db2ChunkType));
