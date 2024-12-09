#pragma once

#include "db2_expression.h"

class db2ScriptDef
{
public:
    void *self{nullptr};

    db2DynArray<db2Expression*> vars; // should be Variables
    db2DynArray<db2Expression*> main; // should be Sequences
};

