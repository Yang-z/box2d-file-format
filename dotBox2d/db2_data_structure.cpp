#include "db2_data_structure.h"
#include "db2_hardware_difference.h"

#include <fstream>

auto dotBox2d::load(char *filePath) -> void
{
    std::ifstream fs{filePath, std::ios::binary};
    if (!fs)
    {
        //error: open file faild
        return;
    }

    fs.read((char *)&(this->raw), sizeof(this->raw));

    if (this->world == nullptr)
        this->world = new dotB2Wrold;
    fs.read((char *)(this->world), sizeof(this->world->raw));

    if (this->bodys == nullptr)
        this->bodys = new dotB2Body[this->raw.countB2Body];
    fs.read((char *)(this->bodys), sizeof(this->bodys->raw) * this->raw.countB2Body);

    if (this->fixtures == nullptr)
        this->fixtures = new dotB2Fixture[this->raw.countB2Fixture];
    fs.read((char *)(this->fixtures), sizeof(this->fixtures->raw) * this->raw.countB2Fixture);

    if (this->vec2s == nullptr)
        this->vec2s = new dotB2Vec2[this->raw.countB2Vec2];
    fs.read((char *)(this->vec2s), sizeof(this->vec2s) * this->raw.countB2Vec2);

    // do endian transfer
}

dotBox2d::~dotBox2d()
{
    if (this->world)
    {
        delete this->world;
        this->world = nullptr;
    }
    if (this->bodys)
    {
        delete[] this->bodys;
        this->bodys = nullptr;
    }
    if (this->fixtures)
    {
        delete[] this->fixtures;
        this->fixtures = nullptr;
    }
    if (this->vec2s)
    {
        delete[] this->vec2s;
        this->vec2s = nullptr;
    }
}
