#pragma once

#include "box2d/box2d.h"

#include "dotBox2d.h"

class db2Transcoder
{
public:
    static auto Transcode(dotBox2d &db2) -> void;

    static auto Transcode_BreakForce_Body(dotBox2d &db2, b2Body *&b2_body, float32_t &break_force) -> void;
    static auto Transcode_BreakForce_Joint(dotBox2d &db2, b2Joint *&b2_joint, float32_t &break_force) -> void;
};
