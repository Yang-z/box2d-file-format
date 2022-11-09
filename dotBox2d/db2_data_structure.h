#pragma once

#include "db2_settings.h"
#include "db2_container.h"

DB2_PRAGMA_PACK_ON

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

    int shape_type{-0};
    float shape_radius{-0.0f};
    // bool shape_oneSided;?

    int shape_vec_begin{-0};
    int shape_vec_end{-0};

    unsigned long long userData{-0};
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

    int fixture_begin{-0};
    int fixture_end{-0};

    unsigned long long userData{-0};
} DB2_NOTE(sizeof(dotB2Body));

ENDIAN_SENSITIVE struct dotB2Joint
{
    int type{0};
    int bodyA{-0}; // index
    int bodyB{-0}; // index
    bool collideConnected{false};

    int para; // index, and for a specified type of joint, the length is fixed.

    unsigned long long userData{-0};

} DB2_NOTE(sizeof(dotB2Joint));

ENDIAN_SENSITIVE struct dotB2Wrold
{
    float gravity_x{0.0f};
    float gravity_y{0.0f};

    int body_begin{-0};
    int body_end{-0};

    int joint_begin{-0};
    int joint_end{-0};
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

    // ENDIAN_SENSITIVE struct Count
    // {
    //     int world{0};
    //     int body{0};
    //     int fixture{0};
    //     int joint{0};
    //     int vec{0};
    // } count DB2_NOTE(sizeof(count));

} DB2_NOTE(sizeof(dotB2Info));

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

    unsigned char head[8]{0xB2, 0x42, 0x32, 0x64, 0x0D, 0x0A, 0x1A, 0x0A};

    struct
    {
        db2Vector<dotB2Info> info{"INFO"};
        db2Vector<dotB2Wrold> world{"WRLD"};
        db2Vector<dotB2Joint> joint{"JOIN"};
        db2Vector<dotB2Body> body{"BODY"};
        db2Vector<dotB2Fixture> fixture{"FXTR"};
        db2Vector<float> vec{"VECT"};
        /* user data */
    } chunks;

    dotBox2d(const char *file = nullptr);

public:
    auto load(const char *filePath) -> void;
    auto save(const char *filePath) -> void;
    auto reverseEndian() -> void;
};
