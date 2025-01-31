#pragma once

#include "db2_chunk.h"

template <typename T, typename T_pfx = void>
class db2Tree : public db2Chunk<T, T_pfx>
{
public:
    using type_type = int32_t;
    using flag_dynamic = void;

    static const db2DynArray<int32_t> end_types;

public:
    auto init(const int32_t type) -> void { std::memcpy(this->type, &type, 4); }

public:
    bool is_end() const { return this->reflector->end_types.has(this->type_i()); }

    auto get_node_list() const -> db2Tree<db2Tree<T, T_pfx>, T_pfx> &
    {
        if (this->is_end())
            return nullval;
        else
            return reinterpret_cast<db2Tree<db2Tree<T, T_pfx>, T_pfx> &>(*this);
    }

    auto get_value_list() const -> db2Tree<T, T_pfx> &
    {
        if (this->is_end())
            return *this;
        else
            nullval;
    }

public: // sub-node
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
        return this->db2DynArray<T>::at(index);
    }

    template <typename... Args>
    auto emplace(const uint32_t index, Args &&...args) -> T &
    {
        assert(this->is_end());
        return this->db2Chunk<T, T_pfx>::emplace(index, std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto emplace_back(Args &&...args) -> T &
    {
        assert(this->is_end());
        return this->db2Chunk<T, T_pfx>::emplace_back(std::forward<Args>(args)...);
    }
};
