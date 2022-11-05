#include "db2_data_structure.h"
#include "db2_hardware_difference.h"

#include <fstream>
#include <algorithm>
#include <assert.h>

#include <boost/pfr.hpp> //reflect

dotBox2d::dotBox2d(const char *file)
{
    if (file)
        dotBox2d::load(file);
}

template <class T>
auto db2Read(std::ifstream &fs, T *&trunk, int &chunkLength, int &count) -> void
{
    int _count = chunkLength / sizeof(T);
    count = _count; /*refrash count*/
    trunk = (T *)malloc(chunkLength);
    if (trunk != nullptr)
        fs.read((char *)(trunk), chunkLength);
}
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

        if (shouldReverseEndian)
            hardwareDifference::reverseEndian((char *)&chunkLength, sizeof(chunkLength));

        fs.read(chunkType, sizeof(chunkType));

        if (std::equal(chunkType, chunkType + 4, this->chunkTypes.INFO))
        {
            fs.read((char *)&(this->info), chunkLength);
            assert(this->info.isLittleEndian == isFileLittleEndian);
        }

        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.WRLD))
        {
            db2Read(fs, this->world, chunkLength, this->info.count.world);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.BODY))
        {
            db2Read(fs, this->body, chunkLength, this->info.count.body);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.FXTR))
        {
            db2Read(fs, this->fixture, chunkLength, this->info.count.fixture);
        }
        else if (std::equal(chunkType, chunkType + 4, this->chunkTypes.VECT))
        {
            db2Read(fs, this->vec2, chunkLength, this->info.count.vec2);
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

template <class T>
auto db2Write(std::ofstream &fs, T *chunk, int count, char const *chunkType) -> void
{
    int chunkLength = sizeof(*chunk) * count;
    fs.write((char *)&chunkLength, sizeof(chunkLength));
    fs.write((char *)chunkType, 4);
    fs.write((char *)chunk, chunkLength);
    /* handle CRC here*/
    int CRC{0};
    fs.write((char *)&CRC, sizeof(CRC));
}
auto dotBox2d::save(const char *filePath) -> void
{
    std::ofstream fs{filePath, std::ios::binary | std::ios::out};
    if (!fs)
        return;

    fs.write((char *)&(this->head), sizeof(this->head));

    db2Write(fs, &this->info, 1, this->chunkTypes.INFO);
    db2Write(fs, this->world, this->info.count.world, this->chunkTypes.WRLD);
    db2Write(fs, this->body, this->info.count.body, this->chunkTypes.BODY);
    db2Write(fs, this->fixture, this->info.count.fixture, this->chunkTypes.FXTR);
    // db2Write(fs, &this->joint, this->info.count.joint, this->chunkTypes.JOIN);
    db2Write(fs, this->vec2, this->info.count.vec2, this->chunkTypes.VECT);

    /* handle user data */

    fs.close();
}

template <class T>
auto dotReverseEndian(T &chunk) -> void
{
    boost::pfr::for_each_field(
        chunk,
        [](auto &field)
        {
            hardwareDifference::reverseEndian((char *)&field, sizeof(chunk));
        });
}
template <class T>
auto dotReverseEndian_batch(T *chunk_arr, int count) -> void
{
    for (int i = 0; i < count; i++)
        dotReverseEndian(chunk_arr[i]);
}
auto dotBox2d::reverseEndian() -> void
{
    this->head[3] = (this->head[3] == 'D') ? 'd' : 'D';
    this->info.isLittleEndian = !this->info.isLittleEndian;
    assert((this->head[3] == 'd') == (this->info.isLittleEndian));

    dotReverseEndian(this->info.count);
    dotReverseEndian_batch(this->world, this->info.count.world);
    dotReverseEndian_batch(this->body, this->info.count.body);
    dotReverseEndian_batch(this->fixture, this->info.count.fixture);
    dotReverseEndian_batch(this->vec2, this->info.count.vec2);
}

dotBox2d::~dotBox2d()
{
    if (this->world)
    {
        free(this->world);
        this->world = nullptr;
    }
    if (this->body)
    {
        free(this->body);
        this->body = nullptr;
    }
    if (this->fixture)
    {
        free(this->fixture);
        this->fixture = nullptr;
    }
    if (this->vec2)
    {
        free(this->vec2);
        this->vec2 = nullptr;
    }
}
