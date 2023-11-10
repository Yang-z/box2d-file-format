#pragma once

#include <fstream>
#include <cstring>

#include <assert.h>

#include "db2_hardware_difference.h"
#include "db2_container.h"
#include "db2_structure_reflector.h"

/*

*/

template <typename T>
class db2Chunk : public db2DynArray<T>
{
public:
    // ENDIAN_SENSITIVE int32_t length{0};  // defined in base
    char type[4]{'N', 'U', 'L', 'L'};
    // T *data{nullptr};  // defined in base
    ENDIAN_SENSITIVE int32_t CRC{0};

private:
    bool isLittleEndian{hardwareDifference::isLittleEndian()};
    db2StructReflector *reflector{nullptr};

public:
    db2Chunk(const char *type = nullptr)
    {
        if (type)
        {
            ::memcpy(this->type, type, 4);
            this->reflector = db2StructReflector::getReflector(this->type);
        }
    }

public:
    /* type-irrelative */
    auto read(std::ifstream &fs, const bool &isLittleEndian) -> void
    {
        assert(this->length == 0);

        // endian
        this->isLittleEndian = isLittleEndian;

        // length
        fs.read((char *)&this->length, sizeof(this->length));
        if (this->isLittleEndian != hardwareDifference::isLittleEndian())
            hardwareDifference::reverseEndian((char *)&this->length, sizeof(this->length));

        // type
        fs.read(this->type, sizeof(this->type));
        this->reflector = db2StructReflector::getReflector(this->type);

        // data
        this->reserve_men(this->length, false);
        fs.read((char *)this->data, this->length);

        // CRC
        fs.read((char *)&(this->CRC), sizeof(this->CRC));
        if (this->isLittleEndian != hardwareDifference::isLittleEndian())
            hardwareDifference::reverseEndian((char *)&this->CRC, sizeof(this->CRC));
        // do check CRC here and before reverseEndian()

        if (this->isLittleEndian != hardwareDifference::isLittleEndian())
        {
            this->reverseEndian();
            this->isLittleEndian = !this->isLittleEndian;
        }
    }

    /* type-irrelative */
    auto write(std::ofstream &fs, bool asLittleEndian = hardwareDifference::isLittleEndian()) -> void
    {
        auto length = this->length;
        auto data = this->data;
        auto CRC = this->CRC;
        if (asLittleEndian != this->isLittleEndian)
        {
            hardwareDifference::reverseEndian((char *)&length, sizeof(length));

            data = (char *)::malloc(this->length);
            ::memcpy(data, this->data, this->length);
            this->reverseEndian(data);

            // recalculate CRC
            hardwareDifference::reverseEndian((char *)&CRC, sizeof(CRC));
        }

        // length
        fs.write((char *)&length, sizeof(length));
        // type
        fs.write((char *)this->type, 4);
        // data
        fs.write((char *)data, this->length);
        // CRC
        fs.write((char *)&CRC, sizeof(CRC));

        if (asLittleEndian != this->isLittleEndian)
            ::free(data);
    }

    /* type-irrelative, since reflector is adopted */
    auto reverseEndian(char *data = nullptr) -> void
    {
        if (!data)
            data = this->data;

        if (this->reflector == nullptr)
            return;

        for (int i = 0; i < this->length / reflector->length; ++i)
            for (int j = 0; j < this->reflector->offsets.size(); ++j)
                hardwareDifference::reverseEndian(
                    data + reflector->length * i + this->reflector->offsets[j],
                    this->reflector->lengths[j]);
    }
};