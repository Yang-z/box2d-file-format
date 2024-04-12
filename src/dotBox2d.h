#pragma once

#include "box2d/box2d.h"

#include "db2_cson.h"
#include "db2_key.h"
#include "db2_structure.h"

class dotBox2d
{
public:
    // uint8_t head[8]{0xB2, 0x42, 0x32, 0x64, 0x0D, 0x0A, 0x1A, 0x0A};
    uint8_t head[8]{
        0xB2,
        'B', '2', uint8_t(HardwareDifference::IsBigEndian() ? 'D' : 'd'),
        0x0D, 0x0A, 0x1A, 0x0A //
    };

    db2Chunks chunks;
    b2World *p_b2w{nullptr};

public:
    dotBox2d(const char *file = nullptr);
    ~dotBox2d();

public:
    auto load(const char *filePath) -> void;
    auto save(const char *filePath, bool asLittleEndian = false) -> void;

    auto decode() -> void;
    auto encode() -> void;
};
