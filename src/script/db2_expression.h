#pragma once

#include <functional>

#include "common/db2_settings.h"
#include "data/db2_key.h"
#include "db2_tensor.h"



class db2Expression
{
public:
    using tensorf = db2Tensor<float32_t>;

public:
    db2Expression *parent;

public:
    int32_t type{db2Key::LITERAL};
    db2DynArray<db2Expression *> args;
    db2Tensor<float32_t> result;

public:
    virtual ~db2Expression();

public:
    std::function<void()> run = []() {};

    auto operator()() -> void;

    auto bind(int32_t type) -> void;
};