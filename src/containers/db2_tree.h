#pragma once

#include "db2_chunk.h"

DB2_PRAGMA_PACK_ON

template <typename T, typename T_pfx>
struct db2Tree : public db2Chunk<T, T_pfx>
{
    using type_type = int32_t;
    using flag_dynamic_nesting = void;
    static constexpr const int32_t type_nondynamic_nesting{0};

    auto init(const int32_t type) -> void { std::memcpy(this->type, &type, 4); }
    bool is_end() { return reinterpret_cast<int32_t &>(this->type) == db2Tree::type_nondynamic_nesting; }

public: // node
    auto at_node(uint32_t index) -> db2Tree &
    {
        assert(!this->is_end());
        return reinterpret_cast<db2Chunk<db2Tree> *>(this)->at(index);
    }

    auto emplace_node(const uint32_t index, const int32_t type) -> db2Tree &
    {
        assert(!this->is_end());
        return reinterpret_cast<db2Chunk<db2Tree> *>(this)->emplace(index, type);
    }

    auto emplace_back_node(const int32_t type) -> db2Tree &
    {
        assert(!this->is_end());
        return reinterpret_cast<db2Chunk<db2Tree> *>(this)->emplace_back(type);
    }

public: // end value
    auto at(uint32_t index) -> T &
    {
        assert(this->is_end());
        return this->db2DynArray::at(index);
    }

    template <typename... Args>
    auto emplace(const uint32_t index, Args &&...args) -> T &
    {
        assert(this->is_end());
        return this->db2Chunk::emplace(index, std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto emplace_back(Args &&...args) -> T &
    {
        assert(this->is_end());
        return this->db2Chunk::emplace_back(std::forward<Args>(args)...);
    }

};

DB2_PRAGMA_PACK_OFF