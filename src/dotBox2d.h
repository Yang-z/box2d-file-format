#pragma once

#include "box2d/box2d.h"

#include "containers/db2_cson.h"
#include "data/db2_key.h"
#include "data/db2_structure.h"

#include "events/db2_contact_listener.h"
#include "events/db2_offstep_listener.h"

// struct b2World_e
// {
//     b2World *world{nullptr};
//     float32_t dt{1.0f / 60.0f};
//     float32_t inv_dt{60.0f};
//     int32_t velocityIterations{6};
//     int32_t positionIterations{2};
// };

class dotBox2d
{
public:
    db2String filePath;

public: // file
    // uint8_t head[8]{0xB2, 0x42, 0x32, 0x64, 0x0D, 0x0A, 0x1A, 0x0A};
    uint8_t head[8]{
        0xB2,
        'B', '2', uint8_t(HardwareDifference::IsBigEndian() ? 'D' : 'd'),
        0x0D, 0x0A, 0x1A, 0x0A //
    };

    db2Chunks chunks;

public: // runtime
    // data
    b2World *p_b2w{nullptr};
    float32_t dt{1.0f / 60.0f};
    float32_t inv_dt{60.0f};
    int32_t velocityIterations{6};
    int32_t positionIterations{2};

    // listeners
    db2ContactListener *p_db2ContactListener{nullptr};
    db2OffstepListener *p_db2OffstepListener{nullptr};

public: // constructors
    dotBox2d(const char *filePath = nullptr);
    ~dotBox2d();

public: // lifecycle
    auto set_file_path(const char *&filePath) -> void;

    auto load(const char *filePath = nullptr) -> void;
    auto save(const char *filePath = nullptr, bool asLittleEndian = false) -> void;

    auto decode() -> void;
    auto encode() -> void;

    auto step() -> void;

public: // getters
    uint32_t world_dict_i();
    db2Dict &world_dict();
};
