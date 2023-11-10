#include "db2_data_structure.h"

#include <fstream>
#include <algorithm>

#include <assert.h>

#include <type_traits> // std::is_same

bool dotB2ChunkType::isRegistered = dotB2ChunkType::registerType();

auto dotB2ChunkType::registerType() -> bool
{
    db2StructReflector::reflect<dotB2Info>(dotB2ChunkType::INFO);
    db2StructReflector::reflect<dotB2Wrold>(dotB2ChunkType::WRLD);
    db2StructReflector::reflect<dotB2Joint>(dotB2ChunkType::JOIN);
    db2StructReflector::reflect<dotB2Body>(dotB2ChunkType::BODY);
    db2StructReflector::reflect<dotB2Fixture>(dotB2ChunkType::FXTR);
    db2StructReflector::reflect<float32_t>(dotB2ChunkType::VECT);

    return true;
}

dotBox2d::dotBox2d(const char *file)
{
    assert(hardwareDifference::check());

    if (!file)
        return;

    dotBox2d::load(file);
}

auto dotBox2d::load(const char *filePath) -> void
{
    std::ifstream fs{filePath, std::ios::binary};
    if (!fs)
        return;

    // read head
    fs.read((char *)&(this->head), sizeof(this->head));

    // confirm endia
    const bool isFileLittleEndian = (this->head[3] == 'd');

    // change to local endian
    this->head[3] = hardwareDifference::isLittleEndian() ? 'd' : 'D';

    // read chunk
    while (fs.peek() != EOF)
    {
        this->chunks.push(new db2Chunk<char>{});
        this->chunks[-1]->read(fs, isFileLittleEndian);
    };

    fs.close();
}

auto dotBox2d::save(const char *filePath, bool asLittleEndian) -> void
{
    std::ofstream fs{filePath, std::ios::binary | std::ios::out};
    if (!fs)
        return;

    this->head[3] = asLittleEndian ? 'd' : 'D';
    fs.write((char *)&(this->head), sizeof(this->head));
    this->head[3] = hardwareDifference::isLittleEndian() ? 'd' : 'D';

    for (auto i = 0; i < this->chunks.size(); ++i)
        this->chunks[i]->write(fs, asLittleEndian);

    fs.close();
}

auto dotBox2d::chunk(const char *type) -> db2Chunk<char> *
{
    for (auto i = 0; i < this->chunks.size(); ++i)
    {
        auto &chunk = this->chunks[i];
        if (std::equal(chunk->type, chunk->type + 4, type))
            return chunk;
    }

    // if not found, create a new one
    this->chunks.push(new db2Chunk<char>{type});
    return this->chunks[-1];
}
