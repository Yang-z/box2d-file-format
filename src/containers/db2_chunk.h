#pragma once

#include <fstream>

#include <boost/crc.hpp> // crc

#include "common/db2_hardware_difference.h"
#include "common/db2_reflector.h"
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

template <typename T, typename T_pfx = void>
class db2Chunk;

template <typename T, typename T_pfx = void>
class db2ChunkStruct;

class db2Chunks;

#define DB2_CHUNK_CONSTRUCTORS(CLS)                                                \
    CLS() = default;                                                               \
    CLS(const CLS &other) = delete;                                                \
    CLS(CLS &&other) = delete;                                                     \
    CLS(const std::initializer_list<typename CLS::value_type> &arg_list) = delete; \
    CLS &operator=(const CLS &other) = delete;                                     \
    CLS &operator=(CLS &&other) = delete;                                          \
    bool operator==(const CLS &other) const = delete;                              \
    bool operator!=(const CLS &other) const = delete;

template <typename T, typename T_pfx>
class db2Chunk : public db2DynArrayWithPrefix<T, T_pfx>
{
public:
    using flag_db2Chunk = void;

public:
    TYPE_IRRELATIVE static auto ReadBytes(char *data, const uint32_t length, std::ifstream &fs, const bool reverseEndian, db2Reflector::pack_info *pack = nullptr, boost::crc_32_type *CRC = nullptr) -> void
    {
        if (data == nullptr || length == 0)
            return;

        fs.read(data, length);

        if (CRC)
            CRC->process_bytes(data, length);

        if (reverseEndian)
            db2Chunk::ReverseEndian(data, length, pack);
    }

    TYPE_IRRELATIVE static auto WriteBytes(char *data, const uint32_t length, std::ofstream &fs, const bool reverseEndian, db2Reflector::pack_info *pack = nullptr, boost::crc_32_type *CRC = nullptr) -> void
    {
        if (data == nullptr || length == 0)
            return;

        if (reverseEndian)
        {
            auto data_r = (char *)std::malloc(length);
            std::memcpy(data_r, data, length);
            data = data_r;
            db2Chunk::ReverseEndian(data, length, pack);
        }

        if (CRC)
            CRC->process_bytes(data, length);

        fs.write(data, length);

        if (reverseEndian)
            std::free(data);
    }

    // type-irrelative, since reflector is adopted
    TYPE_IRRELATIVE static auto ReverseEndian(char *data, const uint32_t length, db2Reflector::pack_info *pack = nullptr) -> void
    {
        if (data == nullptr || length == 0)
            return;

        if (pack == nullptr)
        {
            HardwareDifference::ReverseEndian(data, length);
            return;
        }

        for (int i = 0; i < length / pack->length; ++i)
            for (int j = 0; j < pack->offsets.size(); ++j)
                HardwareDifference::ReverseEndian(
                    data + pack->length * i + pack->offsets[j],
                    pack->lengths[j]);
    }

public:
    ENDIAN_SENSITIVE DEF_IN_BASE(uint32_t length{0});
    char type[4]{0, 0, 0, 0};
    ENDIAN_SENSITIVE DEF_IN_BASE(T *data{nullptr});
    ENDIAN_SENSITIVE uint32_t crc{};

    ENDIAN_SENSITIVE uint32_t length_chunk{0};

    // const bool isLittleEndian{HardwareDifference::IsLittleEndian()};
    db2Reflector *reflector{nullptr};
    db2Chunks *root{nullptr};

    void *runtime = nullptr;

public: // Constructors
    TYPE_IRRELATIVE DB2_CHUNK_CONSTRUCTORS(db2Chunk);

    // only clear up reflected types with the innermost value-types of flat data structures
    // further work is required?
    TYPE_IRRELATIVE virtual ~db2Chunk() override
    {
        if (this->data && this->reflector && this->reflector->get_child(this->type))
        {
            auto &self = *(db2Chunk<db2Chunk<char>> *)this;
            for (auto i = 0; i < self.size(); ++i)
                self[i].~db2Chunk();
        }
    }

public: // initializer
    TYPE_IRRELATIVE auto pre_init(db2Reflector *reflector, db2Chunks *root) -> void
    {
        this->root = root;
        this->reflector = reflector;

        if (this->reflector)
            std::memcpy(this->type, this->reflector->type, sizeof(this->type));
            // for (int i = 0; i < 4; ++i)
            //     if (this->reflector->type[i] != '\0' && this->type[i] == '\0')
            //         this->type[i] = this->reflector->type[i];
    }

    TYPE_IRRELATIVE virtual auto init() -> void {}

public:
    TYPE_IRRELATIVE auto read(std::ifstream &fs, const bool isLittleEndian, boost::crc_32_type *CRC = nullptr) -> void
    {
        assert(this->length == 0);

        const bool reverseEndian = HardwareDifference::IsLittleEndian() != isLittleEndian;

        // length
        db2Chunk::ReadBytes((char *)&this->length_chunk, sizeof(this->length_chunk), fs, reverseEndian, nullptr, CRC);

        // CRC
        if (CRC == nullptr)
            CRC = new boost::crc_32_type{};

        // type
        bool reverseEndian_type = this->reflector != nullptr && this->reflector->is_type_int && reverseEndian;
        db2Chunk::ReadBytes(this->type, sizeof(this->type), fs, reverseEndian_type, nullptr, CRC); // overwrite type with data from file
        if (this->reflector == nullptr)
            this->reflector = db2Reflector::GetReflector(this->type);

        // prefix
        if (this->reflector->prefix)
        {
            this->length_pfx = this->reflector->prefix->length;
            this->reserve_pfx_mem(this->length_pfx);
            db2Chunk::ReadBytes((char *)this->prefix, this->length_pfx, fs, reverseEndian, this->reflector->prefix, CRC);
        }

        // data
        if (this->reflector->get_child(this->type) == nullptr)
        {
            this->length = this->length_chunk - this->length_pfx;
            this->reserve_mem(this->length, false);
            db2Chunk::ReadBytes((char *)this->data, this->length, fs, reverseEndian, this->reflector->value, CRC);
        }
        else
        {
            auto p0 = fs.tellg();
            auto pn = p0;
            while (pn - p0 < this->length_chunk - this->length_pfx)
            {
                auto &child = ((db2Chunk<db2Chunk<char>> *)this)->emplace_back();
                child.read(fs, isLittleEndian, CRC); // recursion
                pn = fs.tellg();
            }
            assert(pn - p0 == this->length_chunk - this->length_pfx);
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
        const bool reverseEndian = HardwareDifference::IsLittleEndian() != asLittleEndian;

        if (this->reflector == nullptr || this->reflector->parent == nullptr)
            this->refresh_length_chunk();

        // length
        db2Chunk::WriteBytes((char *)&this->length_chunk, sizeof(this->length_chunk), fs, reverseEndian, nullptr, CRC);

        // CRC
        if (CRC == nullptr)
            CRC = new boost::crc_32_type{};

        // type
        bool reverseEndian_type = this->reflector != nullptr && this->reflector->is_type_int && reverseEndian;
        db2Chunk::WriteBytes(this->type, sizeof(this->type), fs, reverseEndian_type, nullptr, CRC);

        // prefix
        if (this->reflector->prefix)
            db2Chunk::WriteBytes((char *)this->prefix, this->length_pfx, fs, reverseEndian, this->reflector->prefix, CRC);

        // data
        if (this->reflector->get_child(this->type) == nullptr)
            db2Chunk::WriteBytes(this->data, this->length, fs, reverseEndian, this->reflector->value, CRC);
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

    TYPE_IRRELATIVE auto refresh_length_chunk() -> void
    {
        if (!this->reflector || !this->reflector->get_child(this->type))
        {
            this->length_chunk = this->length + this->length_pfx;
            return;
        }

        this->length_chunk = this->length_pfx;

        auto this_ = (db2Chunk<db2Chunk<char>> *)this;
        for (auto i = 0; i < this_->size(); ++i)
        {
            auto child = this_->data + i;
            child->refresh_length_chunk();
            this->length_chunk += child->length_chunk + 4 * 2; // length and type
        }
    }

public:
    template <typename... Args>
    auto emplace(const uint32_t index, Args &&...args) -> T &
    {
        if constexpr (has_flag_db2Chunk_v<T>) // sub-chunk
        {
            auto &element = this->db2DynArray<T>::emplace(index);
            // element.pre_init(this->reflector->get_child(this->type), this->root);
            reinterpret_cast<db2Chunk<char> &>(element).pre_init(this->reflector->get_child(this->type), this->root);
            element.init(std::forward<Args>(args)...);
            return element;
        }
        else
        {
            return this->db2DynArray<T>::emplace(index, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    auto emplace_back(Args &&...args) -> T &
    {
        if constexpr (has_flag_db2Chunk_v<T>) // sub-chunk
        {
            auto &element = this->db2DynArray<T>::emplace_back();
            // element.pre_init(this->reflector->get_child(this->type), this->root);
            reinterpret_cast<db2Chunk<char> &>(element).pre_init(this->reflector->get_child(this->type), this->root);
            element.init(std::forward<Args>(args)...);
            return element;
        }
        else
        {
            return this->db2DynArray<T>::emplace_back(std::forward<Args>(args)...);
        }
    }

    T &push_back(const T &t) = delete;
};

template <typename T, typename T_pfx>
class db2ChunkStruct : public db2Chunk<T, T_pfx>
{
public:
    TYPE_IRRELATIVE DB2_CHUNK_CONSTRUCTORS(db2ChunkStruct);
    int8_t &type3() { return reinterpret_cast<int8_t &>(this->type[3]); }
};

class db2Chunks : public db2DynArray<db2Chunk<char> *>
{
public:
    ~db2Chunks()
    {
        for (auto i = 0; i < this->size(); ++i)
            delete this->data[i];
    }

public:
    auto operator[](const uint32_t index) const -> db2Chunk<char> & { return *this->db2DynArray<db2Chunk<char> *>::operator[](index); }

    template <typename CK_T>
    auto at() const -> CK_T &
    {
        for (auto i = 0; i < this->size(); ++i)
        {
            auto p_chunk = this->data[i];
            if (p_chunk->reflector->info == &typeid(CK_T))
                return *(CK_T *)p_chunk;
        }
        return nullval;
    }

    template <typename CK_T>
    auto get() -> CK_T &
    {
        auto &chunk = this->at<CK_T>();
        return chunk != nullval ? chunk : this->emplace<CK_T>();
    }

    template <typename CK_T = void, typename default_type = std::conditional_t<std::is_same_v<CK_T, void>, db2Chunk<char>, CK_T>>
    auto emplace() -> default_type &
    {
        auto p_chunk = this->db2DynArray<db2Chunk<char> *>::emplace_back<default_type *>(new default_type());
        p_chunk->pre_init(db2Reflector::GetReflector<CK_T>(), this);
        return *p_chunk;
    }

    db2Chunk<char> *&push_back(const db2Chunk<char> *&t) = delete;
};
