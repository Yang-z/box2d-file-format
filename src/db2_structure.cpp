#include "db2_structure.h"

#include <fstream>

bool db2ChunkType::IsRegistered = db2ChunkType::RegisterType();

auto db2ChunkType::RegisterType() -> bool
{
    db2Reflector::Reflect<CKInfo>(db2ChunkType::INFO);
    db2Reflector::Reflect<CKWorld>(db2ChunkType::WRLD);
    db2Reflector::Reflect<CKJoint>(db2ChunkType::JInT);
    db2Reflector::Reflect<CKBody>(db2ChunkType::BODY);
    db2Reflector::Reflect<CKFixture>(db2ChunkType::FXTR);
    db2Reflector::Reflect<CKShape>(db2ChunkType::SHpE);

    return true;
}
