#include "db2_cson.h"

bool db2ChunkType_CSON::IsRegistered = db2ChunkType_CSON::RegisterType();

auto db2ChunkType_CSON::RegisterType() -> bool
{
    db2Reflector::Reflect<int32_t>(db2ChunkType_CSON::PODI);
    db2Reflector::Reflect<float32_t>(db2ChunkType_CSON::PODF);

    db2Reflector::Reflect<CKDict>(db2ChunkType_CSON::DIcT);
    db2Reflector::Reflect<CKList>(db2ChunkType_CSON::LIsT);
    db2Reflector::Reflect<CKString>(db2ChunkType_CSON::STrG);

    // db2Reflector::Reflect<db2Chunk<db2Dict>>(db2ChunkType_JSON::dIcT);
    // db2Reflector::Reflect<db2Chunk<db2List>>(db2ChunkType_JSON::lIsT);
    // db2Reflector::Reflect<db2Chunk<db2String>>(db2ChunkType_CPPON::sTrG);

    return true;
}