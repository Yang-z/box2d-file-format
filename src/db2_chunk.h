#pragma once

#include <cstring> // std::memcpy
#include <fstream>

#include <boost/crc.hpp> // crc

#include "db2_hardware_difference.h"
#include "db2_reflector.h"
#include "db2_dynarray.h"

/*
class db2Chunk is designed to process flat data structures for file storage.
value type should be a POD (plain old data) type, or another db2Chunk as it's sub-chunk type.
db2Chunk<T> requires being reflected before use, otherwise endian handling and destructing may fault.
db2Chunk<T> could be downgraded to db2Chunk<char> when using, and that's why reflection is required.
type-irrelative functions, as well as reflection mechanism, are adopted,
to make it still functioning even when it's downgraded to db2Chunk<char>.
*/

#define DEF_IN_BASE(def) /* defined in base */

template <typename T>
class db2Chunk;

class db2Chunks;

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
            auto data_r = (char *)std::malloc(length);
            std::memcpy(data_r, data, length);
            data = data_r;
            db2Chunk::ReverseEndian(data, length, reflector);
        }

        if (CRC)
            CRC->process_bytes(data, length);

        fs.write(data, length);

        if (reverseEndian)
            std::free(data);
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

    db2Chunks *root{nullptr};

    db2DynArray<void *> userData;

public:
    db2Chunk() {} // leave default constructor empty

    db2Chunk(const bool auto_reflect)
    {
        if (auto_reflect)
        {
            this->reflector = db2Reflector::GetReflector<db2Chunk<T>>();
            if (this->reflector)
                std::memcpy(this->type, this->reflector->type, 4);
        }
    }

    TYPE_IRRELATIVE db2Chunk(const char *type, const bool auto_reflect = false)
    {
        if (type)
        {
            std::memcpy(this->type, type, 4);
            if (auto_reflect)
                this->reflector = db2Reflector::GetReflector(type);
        }
    }

    TYPE_IRRELATIVE db2Chunk(std::ifstream &fs, const bool isLittleEndian, db2Reflector *reflector = nullptr, boost::crc_32_type *CRC = nullptr)
    {
        this->read(fs, isLittleEndian, reflector, CRC);
    }

    // only clear up reflected types with the innermost value-types of flat data structures
    // further work is required?
    TYPE_IRRELATIVE ~db2Chunk() override
    {
        if (!this->data)
            return;

        if (this->reflector->child)
        {
            auto &self = *(db2Chunk<db2Chunk<char>> *)this;
            for (auto i = 0; i < self.size(); ++i)
                self[i].~db2Chunk();
        }

        // free data, or leave it to base destructor?
        std::free(this->data);
        this->data = nullptr;
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

public:
    // auto add_child() -> T &
    // {
    //     auto &child = this->emplace();
    //     child.reflector = this->reflector->child;
    //     return child;
    // }

    template <typename... Args>
    auto emplace(Args &&...args) -> T &
    {
        auto &element = this->db2DynArray<T>::emplace(args...);
        if constexpr (has_value_type<T>::value) // sub-chunk
        {
            if (element.reflector == nullptr)
                element.reflector = this->reflector->child;
            if (element.root == nullptr)
                element.root = this->root;
        }
        return element;
    }

    T &push(const T &t) = delete;
};

class db2Chunks : public db2DynArray<db2Chunk<char>>
{
public:
    template <typename CK_T>
    auto get() -> CK_T &
    {
        for (auto i = 0; i < this->size(); ++i)
        {
            auto &chunk = this->data[i];
            if (chunk.reflector->id == typeid(CK_T).hash_code())
                return *(CK_T *)&chunk;
        }
        return this->emplace<CK_T>(true);
    }

    template <typename CK_T = db2Chunk<char>, typename... Args>
    auto emplace(Args &&...args) -> CK_T &
    {
        auto &chunk = this->db2DynArray<db2Chunk<char>>::emplace<CK_T>(args...);
        chunk.root = this;
        return chunk;
    }

    db2Chunk<char> &push(const db2Chunk<char> &t) = delete;
};