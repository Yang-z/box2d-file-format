#pragma once

#include "db2_settings.h"

DB2_PRAGMA_PACK_ON

struct dotB2Vec2
{
    float x;
    float y;
} DB2_NOTE(sizeof(dotB2Vec2));

struct dotRange
{
    int start;
    int end;
} DB2_NOTE(sizeof(dotRange));

struct dotB2Fixture
{
    float friction{0.2f};
    float restitution{0.0f};
    float restitutionThreshold{1.0f};
    float density{0.0f};
    bool isSensor{false};
    /* 1 byte gape */
    struct
    {
        unsigned short categoryBits{0x0001};
        unsigned short maskBits{0xFFFF};
        signed short groupIndex{0};
    } filter;

    struct
    {
        int type;
        float radius;

        dotRange vec2s;

        // bool oneSided;?

    } shape;

    unsigned long long userData{0};
} DB2_NOTE(sizeof(dotB2Fixture));

struct dotB2Body
{
    int type{0};
    dotB2Vec2 position{0.0f, 0.0f};
    float angle{0.0f};
    dotB2Vec2 linearVelocity{0.0f, 0.0f};
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

    dotRange fixtures;

    unsigned long long userData{0};
} DB2_NOTE(sizeof(dotB2Body));

struct dotB2Joint
{
    int type;
    /* data */
};

struct dotB2Wrold
{
    dotB2Vec2 gravity{0.0f, 0.0f};

    dotRange bodies;
};

struct dotB2Info
{
    const char packSize{DB2_PACK_SIZE};
    const bool isBigEndian{false};
    DB2_NOTE(sizeof(isBigEndian))

    struct
    {
        const char dotBox2d[3]{0, 0, 0};
        const char box2d[3]{0, 0, 0};
    } version DB2_NOTE(sizeof(version));

    /* 2 bytes gape */

    struct
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

    const unsigned char head[8]{0xB2, 0x42, 0x32, 0x00, 0x0D, 0x0A, 0x1A, 0x0A};
    dotB2Info info{};

    dotB2Wrold *worlds{nullptr};
    dotB2Body *bodies{nullptr};
    dotB2Fixture *fixtures{nullptr};
    dotB2Vec2 *vec2s{nullptr};

    ~dotBox2d();

public:
    auto load(const char *filePath) -> void;
    auto save(const char *filePath) -> void;
};
