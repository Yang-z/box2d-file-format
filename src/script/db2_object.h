#pragma once

#include "common/db2_settings.h"
#include "db2_tensor.h"

class db2Object
{
public:
    static auto Get(void *ptr, int32_t type, int32_t property, db2Tensor<float32_t>& result) -> std::function<void()>;
    static auto Set(void *ptr, int32_t type, int32_t property, db2Tensor<float32_t>& value) -> std::function<void()>;
};
