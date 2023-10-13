#pragma once

#include "box2d/box2d.h"
#include "db2_data_structure.h"

class dotB2Parser
{
public:
    dotBox2d *db2{nullptr};
    b2World *b2w{nullptr};

    // dotB2Parser(){};
    ~dotB2Parser();

    auto parse() -> void;
    auto encode() -> void;
};