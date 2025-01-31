#include "db2_reflector.h"

db2DynArray<db2PackInfo *> db2PackInfo::pack_infos{};
db2DynArray<db2Reflector *> db2Reflector::reflectors{};

// int db2Reflector::caller = db2Reflector::LocalStaticTester();
