#include "db2_nullval.h"

auto db2Nullval::Instance() -> db2Nullval &
{
    static db2Nullval instance{};
    return instance;
}

db2Nullval::db2Nullval()
{
}
