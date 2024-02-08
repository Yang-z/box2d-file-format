#pragma once

#include "box2d/box2d.h"
#include "db2_structure.h"

class db2Decoder
{
public:
    dotBox2d *db2{nullptr};
    b2World *b2w{nullptr};

    // db2Decoder();
    ~db2Decoder();

    auto decode() -> void;
    auto encode() -> void;
};