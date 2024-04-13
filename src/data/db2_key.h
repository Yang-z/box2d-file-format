#pragma once

#include "common/db2_settings.h"

struct db2Key
{
    enum __ : int32_t
    {
        Target = 0,

        World = 1,
        Joint = 2,
        Body = 3,
        Fixture = 4,
        Shape = 5,

        Material = 17,

        BreakForce = 257,
        BreakTorque = 258,

    };
};