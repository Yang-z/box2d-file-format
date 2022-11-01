#include "db2_data_structure.h"
#include "db2_hardware_difference.h"

#include <fstream>
#include <algorithm>
#include <assert.h>

#include <boost/pfr.hpp> //reflect

auto dotBox2d::load(const char *filePath) -> void
{
    std::ifstream fs{filePath, std::ios::binary};
    if (!fs)
        return;

    fs.read((char *)&(this->head), sizeof(this->head));

    bool isFileLittleEndian = (this->head[3] == 'd');
    bool shouldReverseEndian = (isFileLittleEndian != hardwareDifference::isLittleEndian());

    ENDIAN_SENSITIVE int chunkLength{0};
    char chunkType[4]{'N', 'U', 'L', 'L'};
    int CRC{0};

    int count{0};
    while (true)
    {
        fs.read((char *)&chunkLength, sizeof(chunkLength));
        if (fs.eof())
            break;
        fs.read(chunkType, sizeof(chunkType));

        if (shouldReverseEndian)
        {
            hardwareDifference::reverseEndian((char *)&chunkLength, sizeof(chunkLength));
        }

        if (std::equal(chunkType, chunkType + 4, this->chunkTypes.INFO))
        {
            fs.read((char *)&(this->info), chunkLength);
            assert(this->info.isLittleEndian == isFileLittleEndian);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.WRLD))
        {
            count = chunkLength / sizeof(dotB2Wrold);
            this->info.count.world = count;
            if (this->world == nullptr)
                this->world = new dotB2Wrold[count];
            fs.read((char *)(this->world), chunkLength);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.BODY))
        {
            count = chunkLength / sizeof(dotB2Body);
            this->info.count.body = count;
            if (this->body == nullptr)
                this->body = new dotB2Body[count];
            fs.read((char *)(this->body), chunkLength);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.FXTR))
        {
            count = chunkLength / sizeof(dotB2Fixture);
            this->info.count.fixture = count;
            if (this->fixture == nullptr)
                this->fixture = new dotB2Fixture[count];
            fs.read((char *)(this->fixture), chunkLength);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.VECT))
        {
            count = chunkLength / sizeof(dotB2Vec2);
            this->info.count.vec2 = count;
            if (this->vec2 == nullptr)
                this->vec2 = new dotB2Vec2[count];
            fs.read((char *)(this->vec2), chunkLength);
        }
        else
        {
            /* handle user data */
        }

        fs.read((char *)&CRC, sizeof(CRC));
        /* do CRC check here */
    };
    fs.close();

    // do chunk data endian transfer
    if (shouldReverseEndian)
    {
        this->reverseEndian();
    }
}

auto dotBox2d::save(const char *filePath) -> void
{
    std::ofstream fs{filePath, std::ios::binary | std::ios::out};
    if (!fs)
        return;

    fs.write((char *)&(this->head), sizeof(this->head));

    int chunkLength{0};
    int CRC{0};

    chunkLength = sizeof(dotB2Info);
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(this->chunkTypes.INFO), 4);
    fs.write((char *)&(this->info), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Wrold) * this->info.count.world;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(this->chunkTypes.WRLD), 4);
    fs.write((char *)&(this->world), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Body) * this->info.count.body;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(this->chunkTypes.BODY), 4);
    fs.write((char *)&(this->body), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Fixture) * this->info.count.fixture;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(this->chunkTypes.FXTR), 4);
    fs.write((char *)&(this->fixture), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Vec2) * this->info.count.vec2;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(this->chunkTypes.VECT), 4);
    fs.write((char *)&(this->fixture), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    /* handle user data */

    fs.close();
}

auto dotBox2d::reverseEndian() -> void
{
    this->head[3] = (this->head[3] == 'D') ? 'd' : 'D';
    this->info.isLittleEndian = !this->info.isLittleEndian;
    assert((this->head[3] == 'd') == (this->info.isLittleEndian));

#define DB2_FOR(type) \
    for (int i = 0; i < this->info.count.type; i++)

#define DB2_REVERSE(struct)                                                     \
    boost::pfr::for_each_field(                                                 \
        this->struct,                                                           \
        [](auto &field)                                                         \
        {                                                                       \
            hardwareDifference::reverseEndian((char *)&(field), sizeof(field)); \
        });

#define DB2_REVERSE_ALL(arr) \
    DB2_FOR(arr) { DB2_REVERSE(arr[i]); }

    DB2_REVERSE(info.count)

    DB2_REVERSE_ALL(vec2)
    DB2_REVERSE_ALL(fixture)
    DB2_REVERSE_ALL(body)
    DB2_REVERSE_ALL(joint)
    DB2_REVERSE_ALL(world)
}

dotBox2d::~dotBox2d()
{
    if (this->world)
    {
        delete this->world;
        this->world = nullptr;
    }
    if (this->body)
    {
        delete[] this->body;
        this->body = nullptr;
    }
    if (this->fixture)
    {
        delete[] this->fixture;
        this->fixture = nullptr;
    }
    if (this->vec2)
    {
        delete[] this->vec2;
        this->vec2 = nullptr;
    }
}
