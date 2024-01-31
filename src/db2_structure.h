#pragma once

#include "db2_chunk.h"

DB2_PRAGMA_PACK_ON

ENDIAN_SENSITIVE struct dotB2Shape : public db2Chunk<float32_t>
{
    dotB2Shape() : db2Chunk("SHP", false) {}

    int8_t &type3() { return reinterpret_cast<int8_t &>(this->type[3]); }
    float32_t &shape_radius() { return (*this)[0]; }

    int32_t constexpr shape_extend() { return 1; } // ... extend data
};

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

    int32_t shape{-1}; // one shape per fixture

    // int32_t extra{-1};

} DB2_NOTE(sizeof(dotB2Fixture) == 28);

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

    int32_t fixtureList{-1}; // one body could have multiple fixtures
    int32_t fixtureCount{0};

    // int32_t extra{-1};

} DB2_NOTE(sizeof(dotB2Body) == 56);

ENDIAN_SENSITIVE struct dotB2Joint : public db2Chunk<float32_t>
{
    dotB2Joint() : db2Chunk("JIN", false) {}

    int8_t &type3() { return reinterpret_cast<int8_t &>(this->type[3]); }

    int32_t &bodyA() { return reinterpret_cast<int32_t &>((*this)[0]); } // index
    int32_t &bodyB() { return reinterpret_cast<int32_t &>((*this)[1]); } // index
    bool &collideConnected() { return ((bool *)&((*this)[2]))[hardwareDifference::IsBigEndian() ? 0 : 3]; }

    int32_t constexpr extend() { return 3; } // ... extend data
};

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
    static constexpr const char JInT[4]{'J', 'I', 'n', 'T'};
    static constexpr const char BODY[4]{'B', 'O', 'D', 'Y'};
    static constexpr const char FXTR[4]{'F', 'X', 'T', 'R'};
    static constexpr const char SHpE[4]{'S', 'H', 'p', 'E'};

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
        CK_T *chunk = new CK_T(true);
        this->chunks.push((db2Chunk<char> *)chunk);
        return *chunk;
    }
};
