#pragma once

#include <functional>

#include "box2d/box2d.h"

#include "containers/db2_dynarray.h"

class db2OffstepListener
{
public:
    db2DynArray<std::function<bool()>> onPreStep{};
    db2DynArray<std::function<bool()>> onPostStep{};

public:
    auto PreStep() -> void;
    auto PostStep() -> void;
};