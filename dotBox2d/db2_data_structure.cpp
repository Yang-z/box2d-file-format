#include "db2_data_structure.h"
#include "db2_hardware_difference.h"

#include <fstream>
#include <algorithm>

struct chunkTypes
{
    char BXTW[4]{'B','X', 'T', 'W'};
    char WRLD[4]{'W','R', 'L', 'D'};
    char BODY[4]{'B','O', 'D', 'Y'};
    char FXTR[4]{'F','X', 'T', 'R'};
    char VECT[4]{'V','E', 'C', 'T'};

} ChunkTypes;

// care about Data Structure Alignment?
auto dotBox2d::load(char *filePath) -> void
{
    std::ifstream fs{filePath, std::ios::binary};
    if (!fs) return;

    fs.read((char *)&(this->head), sizeof(this->head));
    
    int chunkLength{0};
    char chunkType[4]{'N','U','L', 'L'};
    int CRC{0};

    while(true){
        fs.read((char *)&chunkLength, sizeof(chunkLength));
        if(fs.eof()) break;
        fs.read(chunkType, sizeof(chunkType) * 4);

        if(std::equal(chunkType, chunkType + 4, ChunkTypes.BXTW))
        {
            fs.read((char *)&(this->version), chunkLength);

        }else if(std::equal(chunkType, chunkType + 4, ChunkTypes.WRLD))
        {
            this->countWorld = chunkLength/sizeof(dotB2Wrold);
            if (this->worlds == nullptr)
                this->worlds = new dotB2Wrold[this->countWorld];
            fs.read((char *)(this->worlds), chunkLength);

        }else if(std::equal(chunkType, chunkType + 4, ChunkTypes.BODY))
        {
            this->countbody = chunkLength/sizeof(dotB2Body);
            if (this->bodies == nullptr)
                this->bodies = new dotB2Body[this->countbody];
            fs.read((char *)(this->bodies), chunkLength);

        }else if(std::equal(chunkType, chunkType + 4, ChunkTypes.FXTR))
        {
            this->countFixture = chunkLength/sizeof(dotB2Fixture);
            if (this->fixtures == nullptr)
                this->fixtures = new dotB2Fixture[this->countFixture];
            fs.read((char *)(this->fixtures), chunkLength);

        }else if(std::equal(chunkType, chunkType + 4, ChunkTypes.VECT))
        {
            this->countVec2 = chunkLength/sizeof(dotB2Vec2);
            if (this->vec2s == nullptr)
                this->vec2s = new dotB2Vec2[this->countVec2];
            fs.read((char *)(this->vec2s), chunkLength);

        }else
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
auto dotBox2d::save(char *filePath) -> void
{
    std::ofstream fs{filePath, std::ios::binary|std::ios::out};
    if (!fs)
        return;

    fs.write((char *)&(this->head), sizeof(this->head));
    
    int chunkLength{0};
    int CRC{0};
    
    chunkLength = sizeof(this->version);
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(ChunkTypes.BXTW), 4);
    fs.write((char *)&(this->version), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Wrold) * this->countWorld;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(ChunkTypes.WRLD), 4);
    fs.write((char *)&(this->worlds), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Body) * this->countbody;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(ChunkTypes.BODY), 4);
    fs.write((char *)&(this->bodies), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Fixture) * this->countFixture;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(ChunkTypes.FXTR), 4);
    fs.write((char *)&(this->fixtures), chunkLength);
    /* handle CRC here*/
    fs.write((char *)&CRC, 4);

    chunkLength = sizeof(dotB2Vec2) * this->countVec2;
    fs.write((char *)&chunkLength, 4);
    fs.write((char *)(ChunkTypes.VECT), 4);
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
