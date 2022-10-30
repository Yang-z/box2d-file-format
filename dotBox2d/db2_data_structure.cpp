#include "db2_data_structure.h"
#include "db2_hardware_difference.h"

#include <fstream>
#include <algorithm>
// #include <arpa/inet.h>

// care about Data Structure Alignment?
auto dotBox2d::load(const char *filePath) -> void
{
    std::ifstream fs{filePath, std::ios::binary};
    if (!fs)
        return;

    fs.read((char *)&(this->head), sizeof(this->head));

    int chunkLength{0};
    char chunkType[4]{'N', 'U', 'L', 'L'};
    int CRC{0};

    int count{0};
    while (true)
    {
        fs.read((char *)&chunkLength, sizeof(chunkLength));
        if (fs.eof())
            break;
        fs.read(chunkType, sizeof(chunkType));

        if (std::equal(chunkType, chunkType + 4, this->chunkTypes.INFO))
        {
            fs.read((char *)&(this->info), chunkLength);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.WRLD))
        {
            count = chunkLength / sizeof(dotB2Wrold);
            this->info.count.world = count;
            if (this->worlds == nullptr)
                this->worlds = new dotB2Wrold[count];
            fs.read((char *)(this->worlds), chunkLength);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.BODY))
        {
            count = chunkLength / sizeof(dotB2Body);
            this->info.count.body = count;
            if (this->bodies == nullptr)
                this->bodies = new dotB2Body[count];
            fs.read((char *)(this->bodies), chunkLength);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.FXTR))
        {
            count = chunkLength / sizeof(dotB2Fixture);
            this->info.count.fixture = count;
            if (this->fixtures == nullptr)
                this->fixtures = new dotB2Fixture[count];
            fs.read((char *)(this->fixtures), chunkLength);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.VECT))
        {
            count = chunkLength / sizeof(dotB2Vec2);
            this->info.count.vec2 = count;
            if (this->vec2s == nullptr)
                this->vec2s = new dotB2Vec2[count];
            fs.read((char *)(this->vec2s), chunkLength);
        }
        else
        {
            /* handle user data */
        }

        fs.read((char *)&CRC, sizeof(CRC));
        /* do CRC check here */
    };
    fs.close();
    /* do endian transfer? */
}

// care about Data Structure Alignment?
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
    fs.write((char *)&(this->worlds), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Body) * this->info.count.body;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(this->chunkTypes.BODY), 4);
    fs.write((char *)&(this->bodies), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Fixture) * this->info.count.fixture;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(this->chunkTypes.FXTR), 4);
    fs.write((char *)&(this->fixtures), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Vec2) * this->info.count.vec2;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(this->chunkTypes.VECT), 4);
    fs.write((char *)&(this->fixtures), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    /* handle user data */

    fs.close();
}

dotBox2d::~dotBox2d()
{
    if (this->worlds)
    {
        delete this->worlds;
        this->worlds = nullptr;
    }
    if (this->bodies)
    {
        delete[] this->bodies;
        this->bodies = nullptr;
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
