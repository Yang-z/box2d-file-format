#include "db2_data_structure.h"

#include <fstream>
#include <algorithm>

#include <assert.h>

#include <boost/pfr.hpp> //reflect

#include "db2_hardware_difference.h"

dotBox2d::dotBox2d(const char *file)
{
    if (file)
        dotBox2d::load(file);
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
    uint32_t CRC{0};

    while (true)
    {
        fs.read((char *)&chunkLength, sizeof(chunkLength));
        if (fs.eof())
            break;

        if (shouldReverseEndian)
            hardwareDifference::reverseEndian((char *)&chunkLength, sizeof(chunkLength));

        fs.read(chunkType, sizeof(chunkType));

        boost::pfr::for_each_field(
            this->chunks,
            [&fs, &chunkType, &chunkLength, &CRC](auto &chunk)
            {
                if (std::equal(chunkType, chunkType + 4, chunk.tag))
                {
                    chunk.read(fs, chunkLength);
                    fs.read((char *)&CRC, sizeof(CRC));
                    /* do CRC check here */
                }
            });
    };
    fs.close();

    assert(this->chunks.info[0].isLittleEndian == isFileLittleEndian);

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

    boost::pfr::for_each_field(
        this->chunks,
        [&fs](auto &chunk)
        {
            if (chunk.size <= 0)
                return;
            int chunkLength = sizeof(chunk[0]) * chunk.size;
            fs.write((char *)&chunkLength, sizeof(chunkLength));
            fs.write((char *)chunk.tag, 4);
            // fs.write((char *)&chunk[0], chunkLength);
            chunk.write(fs);
            /* handle CRC here*/
            int CRC{0};
            fs.write((char *)&CRC, sizeof(CRC));
        });

    fs.close();
}

auto dotBox2d::reverseEndian() -> void
{
    this->head[3] = (this->head[3] == 'D') ? 'd' : 'D';
    this->chunks.info[0].isLittleEndian = !this->chunks.info[0].isLittleEndian;
    assert((this->head[3] == 'd') == (this->chunks.info[0].isLittleEndian));

    boost::pfr::for_each_field(
        this->chunks,
        [this](auto &chunk)
        {
            for (int i = 0; i < chunk.size; i++)
            {
                if (std::equal(chunk.tag, chunk.tag + 4, this->chunks.info.tag))
                    return;
                boost::pfr::for_each_field(
                    chunk[i],
                    [](auto &field)
                    {
                        hardwareDifference::reverseEndian((char *)&field, sizeof(field));
                    });
            }
        });
}
