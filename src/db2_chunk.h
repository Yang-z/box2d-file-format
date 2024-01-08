#pragma once

#include <fstream>
#include <cstring>

#include <assert.h>

#include <boost/crc.hpp>

#include "db2_hardware_difference.h"
#include "db2_dynarray.h"
#include "db2_structure_reflector.h"


template <typename T>
class db2Chunk : public db2DynArray<T>
{
public:
    ENDIAN_SENSITIVE; // int32_t length{0};  // defined in base
    char type[4]{'N', 'U', 'L', 'L'};
    ENDIAN_SENSITIVE; // T *data{nullptr};  // defined in base
    ENDIAN_SENSITIVE int32_t CRC{0};

private:
    bool isLittleEndian{hardwareDifference::IsLittleEndian()};
    db2StructReflector *reflector{nullptr};

public:
    db2Chunk(const char *type = nullptr)
    {
        if (type)
        {
            ::memcpy(this->type, type, 4);
            this->reflector = db2StructReflector::GetReflector(this->type);
        }
    }

    db2Chunk(std::ifstream &fs, const bool isLittleEndian)
    {
        this->read(fs, isLittleEndian);
    }

public:
    /* type-irrelative */
    auto read(std::ifstream &fs, const bool isLittleEndian) -> void
    {
        assert(this->length == 0);

        // endian
        this->isLittleEndian = isLittleEndian;

        // length
        fs.read((char *)&this->length, sizeof(this->length));
        if (this->isLittleEndian != hardwareDifference::IsLittleEndian())
            hardwareDifference::ReverseEndian((char *)&this->length, sizeof(this->length));

        // type
        fs.read(this->type, sizeof(this->type));
        this->reflector = db2StructReflector::GetReflector(this->type);

        // data
        this->reserve_men(this->length, false);
        fs.read((char *)this->data, this->length);

        // CRC
        fs.read((char *)&(this->CRC), sizeof(this->CRC));
        if (this->isLittleEndian != hardwareDifference::IsLittleEndian())
            hardwareDifference::ReverseEndian((char *)&this->CRC, sizeof(this->CRC));

        // do check CRC here and before reverseEndian()
        assert(this->calculateCRC() == this->CRC);

        // handle endian of data
        if (this->isLittleEndian != hardwareDifference::IsLittleEndian())
        {
            this->reverseEndian();
            this->isLittleEndian = !this->isLittleEndian;
        }
    }

    /* type-irrelative */
    auto write(std::ofstream &fs, const bool asLittleEndian = hardwareDifference::IsLittleEndian()) -> void
    {
        auto length = this->length;
        if (asLittleEndian != this->isLittleEndian)
            hardwareDifference::ReverseEndian((char *)&length, sizeof(length));

        auto data = this->data;
        if (asLittleEndian != this->isLittleEndian)
        {
            data = (char *)::malloc(this->length);
            ::memcpy(data, this->data, this->length);
            this->reverseEndian((char *)data);
        }

        // calculate CRC
        auto CRC = this->calculateCRC(data);
        if (asLittleEndian != this->isLittleEndian)
            hardwareDifference::ReverseEndian((char *)&CRC, sizeof(CRC));

        // length
        fs.write((char *)&length, sizeof(length));
        // type
        fs.write((char *)this->type, 4);
        // data
        fs.write((char *)data, this->length);
        // CRC
        fs.write((char *)&CRC, sizeof(CRC));

        if (data != this->data)
            ::free(data);
    }

    /* type-irrelative, since reflector is adopted */
    auto reverseEndian(char *data = nullptr) -> void
    {
        if (!data)
            data = (char *)this->data;

        if (this->reflector == nullptr)
            return;

        for (int i = 0; i < this->length / reflector->length; ++i)
            for (int j = 0; j < this->reflector->offsets.size(); ++j)
                hardwareDifference::ReverseEndian(
                    data + reflector->length * i + this->reflector->offsets[j],
                    this->reflector->lengths[j]);
    }

    /* type-irrelative */
    auto calculateCRC(const void *data = nullptr) -> const uint32_t
    {
        if (!data)
            data = (void *)this->data;

        // boost::crc_optimal<32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true> crc;
        boost::crc_32_type crc;
        crc.process_bytes(data, this->length);
        return crc.checksum();
    }
};