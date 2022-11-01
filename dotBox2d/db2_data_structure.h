#pragma once

#include "db2_settings.h"

DB2_PRAGMA_PACK_ON

// struct dotRange
// {
//     int start;
//     int end;
// } DB2_NOTE(sizeof(dotRange));

ENDIAN_SENSITIVE struct dotB2Vec2
{
    float x;
    float y;
} DB2_NOTE(sizeof(dotB2Vec2));

ENDIAN_SENSITIVE struct dotB2Fixture
{
    float friction{0.2f};
    float restitution{0.0f};
    float restitutionThreshold{1.0f};
    float density{0.0f};
    bool isSensor{false};

    /* 1 byte gape */

    unsigned short filter_categoryBits{0x0001};
    unsigned short filter_maskBits{0xFFFF};
    signed short filter_groupIndex{0};

    int shape_type;
    float shape_radius;
    // bool shape_oneSided;?

    int shape_vec2s_start;
    int shape_vec2s_end;

    unsigned long long userData{0};
} DB2_NOTE(sizeof(dotB2Fixture));

ENDIAN_SENSITIVE struct dotB2Body
{
    int type{0};
    float position_x{0.0f};
    float position_y{0.0f};
    float angle{0.0f};
    float linearVelocity_x{0.0f};
    float linearVelocity_y{0.0f};
    float angularVelocity{0.0f};
    float linearDamping{0.0f};
    float angularDamping{0.0f};
    bool allowSleep{true};
    bool awake{true};
    bool fixedRotation{false};
    bool bullet{false};
    bool enabled{true};
    /* 3 bytes gape*/
    float gravityScale{1.0f};

    int fixture_start;
    int fixture_end;

    unsigned long long userData{0};
} DB2_NOTE(sizeof(dotB2Body));

ENDIAN_SENSITIVE struct dotB2Joint
{
    int type;
    /* data */
} DB2_NOTE(sizeof(dotB2Joint));

ENDIAN_SENSITIVE struct dotB2Wrold
{
    float gravity_x{0.0f};
    float gravity_y{0.0f};

    int body_start;
    int body_end;

    int joint_start;
    int joint_end;
} DB2_NOTE(sizeof(dotB2Wrold));

struct dotB2Info
{
    const char packSize{DB2_PACK_SIZE};
    bool isLittleEndian{true};

    struct
    {
        const char dotBox2d[3]{0, 0, 1};
        const char box2d[3]{2, 4, 1};
    } version DB2_NOTE(sizeof(version));

    ENDIAN_SENSITIVE struct Count
    {
        int world{0};
        int body{0};
        int fixture{0};
        int joint{0};
        int vec2{0};
    } count DB2_NOTE(sizeof(count));

    // struct
    // {
    //     int world;
    //     int body;
    //     int fixture;
    //     int vec2;
    // }loc;

} DB2_NOTE(sizeof(dotB2Info));

DB2_PRAGMA_PACK_OFF

class dotBox2d
{
    struct ChunkTypes
    {
        const char INFO[4]{'I', 'N', 'F', 'O'};
        const char WRLD[4]{'W', 'R', 'L', 'D'};
        const char BODY[4]{'B', 'O', 'D', 'Y'};
        const char FXTR[4]{'F', 'X', 'T', 'R'};
        const char VECT[4]{'V', 'E', 'C', 'T'};
    } DB2_NOTE(sizeof(ChunkTypes));

    const dotBox2d::ChunkTypes chunkTypes{};

    unsigned char head[8]{0xB2, 0x42, 0x32, 0x64, 0x0D, 0x0A, 0x1A, 0x0A};
    dotB2Info info{};

    dotB2Wrold *world{nullptr};
    dotB2Body *body{nullptr};
    dotB2Fixture *fixture{nullptr};
    dotB2Body *joint{nullptr};
    dotB2Vec2 *vec2{nullptr};

    ~dotBox2d();

public:
    auto load(const char *filePath) -> void;
    auto save(const char *filePath) -> void;
    auto reverseEndian() -> void;
};
