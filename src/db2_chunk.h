#pragma once

#include <fstream>
#include <cstring>

#include <assert.h>

#include <boost/crc.hpp>

#include "db2_hardware_difference.h"
#include "db2_dynarray.h"
#include "db2_reflector.h"

/* */

#define DEF_IN_BASE(def) /* defined in base */

template <typename T>
class db2Chunk : public db2DynArray<T>
{

public:
    TYPE_IRRELATIVE static auto ReadBytes(char *data, const int32_t length, std::ifstream &fs, const bool reverseEndian, db2Reflector *reflector = nullptr, boost::crc_32_type *CRC = nullptr) -> void
    {
        if (data == nullptr || length == 0)
            return;

        fs.read(data, length);

        if (CRC)
            CRC->process_bytes(data, length);

        if (reverseEndian)
            db2Chunk::ReverseEndian(data, length, reflector);
    }

    TYPE_IRRELATIVE static auto WriteBytes(char *data, const int32_t length, std::ofstream &fs, const bool reverseEndian, db2Reflector *reflector = nullptr, boost::crc_32_type *CRC = nullptr) -> void
    {
        if (data == nullptr || length == 0)
            return;

        if (reverseEndian)
        {
            auto data_r = (char *)::malloc(length);
            ::memcpy(data_r, data, length);
            data = data_r;
            db2Chunk::ReverseEndian(data, length, reflector);
        }

        if (CRC)
            CRC->process_bytes(data, length);

        fs.write(data, length);

        if (reverseEndian)
            ::free(data);
    }

    // type-irrelative, since reflector is adopted
    TYPE_IRRELATIVE static auto ReverseEndian(char *data, const int32_t length, db2Reflector *reflector = nullptr) -> void
    {
        if (data == nullptr || length == 0)
            return;

        if (reflector == nullptr)
        {
            hardwareDifference::ReverseEndian(data, length);
            return;
        }

        for (int i = 0; i < length / reflector->length; ++i)
            for (int j = 0; j < reflector->offsets.size(); ++j)
                hardwareDifference::ReverseEndian(
                    data + reflector->length * i + reflector->offsets[j],
                    reflector->lengths[j]);
    }

public:
    ENDIAN_SENSITIVE DEF_IN_BASE(int32_t length{0});
    char type[4]{'N', 'U', 'L', 'L'};
    ENDIAN_SENSITIVE DEF_IN_BASE(T *data{nullptr});
    ENDIAN_SENSITIVE uint32_t crc{};

    ENDIAN_SENSITIVE int32_t length_chunk{0};

    // const bool isLittleEndian{hardwareDifference::IsLittleEndian()};
    db2Reflector *reflector{nullptr};

public:
    db2Chunk()
    {
        this->reflector = db2Reflector::GetReflector<db2Chunk<T>>();
        ::memcpy(this->type, this->reflector->type, 4);
    }

    TYPE_IRRELATIVE db2Chunk(const char *type, db2Reflector *reflector = nullptr)
    {
        if (type)
            ::memcpy(this->type, type, 4);

        this->reflector = reflector ? reflector : (type ? db2Reflector::GetReflector(type) : nullptr);
    }

    TYPE_IRRELATIVE db2Chunk(std::ifstream &fs, const bool isLittleEndian, db2Reflector *reflector = nullptr, boost::crc_32_type *CRC = nullptr)
    {
        this->read(fs, isLittleEndian, reflector, CRC);
    }

public:
    TYPE_IRRELATIVE auto read(std::ifstream &fs, const bool isLittleEndian, db2Reflector *reflector = nullptr, boost::crc_32_type *CRC = nullptr) -> void
    {
        assert(this->length == 0);

        const bool reverseEndian = hardwareDifference::IsLittleEndian() != isLittleEndian;

        // length
        db2Chunk::ReadBytes((char *)&this->length_chunk, sizeof(this->length_chunk), fs, reverseEndian, nullptr, CRC);

        // CRC
        if (CRC == nullptr)
            CRC = new boost::crc_32_type{};

        // type
        db2Chunk::ReadBytes(this->type, sizeof(this->type), fs, false, nullptr, CRC);
        this->reflector = reflector ? reflector : db2Reflector::GetReflector(this->type);

        // data
        if (this->reflector->child == nullptr)
        {
            this->length = this->length_chunk;
            this->reserve_men(this->length, false);
            db2Chunk::ReadBytes((char *)this->data, this->length, fs, reverseEndian, this->reflector, CRC);
        }
        else
        {
            auto p0 = fs.tellg();
            auto pn = p0;
            while (pn - p0 < this->length_chunk)
            {
                ((db2Chunk<db2Chunk<char>> *)this)->emplace(fs, isLittleEndian, this->reflector->child, CRC);
                pn = fs.tellg();
            }
            assert(pn - p0 == this->length_chunk);
        }

        // CRC
        if (this->reflector->parent == nullptr)
        {
            db2Chunk::ReadBytes((char *)&(this->crc), sizeof(this->crc), fs, reverseEndian, nullptr, nullptr);
            assert(CRC->checksum() == this->crc);
            delete CRC;
        }
    }

    TYPE_IRRELATIVE auto write(std::ofstream &fs, const bool asLittleEndian, boost::crc_32_type *CRC = nullptr) -> void
    {
        const bool reverseEndian = hardwareDifference::IsLittleEndian() != asLittleEndian;

        if (this->reflector == nullptr || this->reflector->parent == nullptr)
        {
            this->refreshReflector();
            this->refreshLengthChunk();
        }

        // length
        db2Chunk::WriteBytes((char *)&this->length_chunk, sizeof(this->length_chunk), fs, reverseEndian, nullptr, CRC);

        // CRC
        if (CRC == nullptr)
            CRC = new boost::crc_32_type{};

        // type
        db2Chunk::WriteBytes(this->type, sizeof(this->type), fs, false, nullptr, CRC);

        // data
        if (this->reflector->child == nullptr)
            db2Chunk::WriteBytes(this->data, this->length, fs, reverseEndian, this->reflector, CRC);
        else
        {
            auto &self = *(db2Chunk<db2Chunk<char>> *)this;
            for (auto i = 0; i < self.size(); ++i)
            {
                auto &child = self[i];
                child.write(fs, asLittleEndian, CRC);
            }
        }

        // calculate CRC
        if (this->reflector->parent == nullptr)
        {
            auto crc = CRC->checksum();
            db2Chunk::WriteBytes((char *)&crc, sizeof(crc), fs, reverseEndian, nullptr, nullptr);
            delete CRC;
        }
    }

    TYPE_IRRELATIVE auto refreshReflector() -> void
    {
        this->reflector = this->reflector ? this->reflector : db2Reflector::GetReflector(this->type);

        if (this->reflector == nullptr)
            return;

        if (this->reflector->child != nullptr)
        {
            auto &self = *(db2Chunk<db2Chunk<char>> *)this;
            for (auto i = 0; i < self.size(); ++i)
            {
                auto &child = self[i];
                child.reflector = this->reflector->child;
                child.refreshReflector();
            }
        }
    }

    TYPE_IRRELATIVE auto refreshLengthChunk() -> void
    {
        if (this->reflector == nullptr)
        {
            this->length_chunk = this->length;
        }
        else
        {
            if (this->reflector->child != nullptr)
            {
                this->length_chunk = 0;

                auto &self = *(db2Chunk<db2Chunk<char>> *)this;
                for (auto i = 0; i < self.size(); ++i)
                {
                    auto &child = self[i];
                    // child.reflector = this->reflector->child;
                    child.refreshLengthChunk();

                    this->length_chunk += child.length_chunk + 2 * 4;
                }
            }
            else
            {
                this->length_chunk = this->length;
            }
        }
    }
};