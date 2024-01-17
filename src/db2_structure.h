#pragma once

#include "db2_settings.h"
#include "db2_hardware_difference.h"
#include "db2_reflector.h"
#include "db2_dynarray.h"
#include "db2_chunk.h"

DB2_PRAGMA_PACK_ON

// ENDIAN_SENSITIVE struct dotB2Shape
// {
//     int32_t _length{2};

//     char type[4]{'S', 'P', 0, 0};
//     float32_t shape_radius{-0.0f};

//     // ... extend data
// };

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

    int32_t shape{-1};
    // int32_t shape_type{-1};
    // float32_t shape_radius{-0.0f};
    // int32_t shape_extend{-1};

    int32_t extraDict{-1};

    uint64_t userData{0};
} DB2_NOTE(sizeof(dotB2Fixture) == 40);

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

    int32_t fixtureList{-1};
    int32_t fixtureCount{0};

    int32_t extraDict{-1};
    /* 4 bytes gape*/
    uint64_t userData{0};
} DB2_NOTE(sizeof(dotB2Body) == 72);

ENDIAN_SENSITIVE struct dotB2Joint
{
    int32_t type{0};
    int32_t bodyA{-1}; // index
    int32_t bodyB{-1}; // index
    bool collideConnected{false};
    /* 3 bytes gape*/
    int32_t extend; // index, and for a specified type of joint, the length is fixed.

    int32_t extraDict{-1};

    uint64_t userData{0};

} DB2_NOTE(sizeof(dotB2Joint) == 32);

ENDIAN_SENSITIVE struct dotB2Wrold
{
    float32_t gravity_x{0.0f};
    float32_t gravity_y{0.0f};

    int32_t bodyList{0};
    int32_t bodyCount{0};

    int32_t jointList{0};
    int32_t jointCount{0};
} DB2_NOTE(sizeof(dotB2Wrold) == 24);

struct dotB2Info
{
    const uint8_t packSize{DB2_PACK_SIZE};
    const uint8_t notUsed{0}; // bool isLittleEndian{hardwareDifference::IsLittleEndian()};

    uint8_t ver_dotBox2d_0{0};
    uint8_t ver_dotBox2d_1{0};
    uint8_t ver_dotBox2d_2{1};

    uint8_t ver_box2d_0{2};
    uint8_t ver_box2d_1{4};
    uint8_t ver_box2d_2{1};

} DB2_NOTE(sizeof(dotB2Info) == 8);

DB2_PRAGMA_PACK_OFF

struct db2ChunkType
{
    static constexpr const char INFO[4]{'I', 'N', 'F', 'O'};
    static constexpr const char WRLD[4]{'W', 'R', 'L', 'D'};
    static constexpr const char JINT[4]{'J', 'I', 'N', 'T'};
    static constexpr const char BODY[4]{'B', 'O', 'D', 'Y'};
    static constexpr const char FXTR[4]{'F', 'X', 'T', 'R'};
    static constexpr const char SHaP[4]{'S', 'H', 'a', 'P'};

    static constexpr const char JInX[4]{'J', 'I', 'n', 'X'};

    static constexpr const char DIcT[4]{'D', 'I', 'c', 'T'};
    static constexpr const char LIsT[4]{'L', 'I', 's', 'T'};
    static constexpr const char CHAR[4]{'C', 'H', 'A', 'R'};

    static constexpr const char dIct[4]{'d', 'I', 'c', 't'};
    static constexpr const char lIst[4]{'l', 'I', 's', 't'};
    static constexpr const char cHAR[4]{'c', 'H', 'A', 'R'};

    static bool IsRegistered;
    static bool RegisterType();

} DB2_NOTE(sizeof(db2ChunkType));

class dotBox2d
{
public:
    // uint8_t head[8]{0xB2, 0x42, 0x32, 0x64, 0x0D, 0x0A, 0x1A, 0x0A};
    uint8_t head[8]{
        0xB2,
        'B', '2', uint8_t(hardwareDifference::IsBigEndian() ? 'D' : 'd'),
        0x0D, 0x0A, 0x1A, 0x0A};

    db2DynArray<db2Chunk<char> *> chunks;

    dotBox2d(const char *file = nullptr);
    ~dotBox2d();

public:
    auto load(const char *filePath) -> void;
    auto save(const char *filePath, bool asLittleEndian = false) -> void;

public:
    template <typename CK_T>
    auto get() -> CK_T &
    {
        for (auto i = 0; i < this->chunks.size(); ++i)
        {
            auto &p_chunk = this->chunks[i];
            if (p_chunk->reflector->id == typeid(CK_T).hash_code())
                return *(CK_T *)p_chunk;
        }
        return this->add<CK_T>();
    }

    template <typename CK_T>
    auto add() -> CK_T &
    {
        CK_T *chunk = new CK_T();
        this->chunks.push((db2Chunk<char> *)chunk);
        return *chunk;
    }
};
