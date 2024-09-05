#pragma once

#include <functional>

#include "common/db2_settings.h"
#include "data/db2_key.h"
#include "db2_tensor.h"

class db2Function
{
    using tensorf = db2Tensor<float32_t>;

public:
    int32_t type{db2Key::Literal};
    db2DynArray<db2Function> args;
    db2Tensor<float32_t> result;

public:
    std::function<void()> func = []() {};

    auto operator()() -> void;

    auto bind(int32_t type) -> void;
};