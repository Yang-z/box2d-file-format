#pragma once

#include <cassert> // assert static_assert
#include <cstdlib> // std::malloc std::free std::realloc
#include <cmath>   // std::log2 std::pow

#include <functional>

#include "db2_settings.h"

/*
It's a std::vector-like container.
Vector is a concept in maths or physics, and it's not a suitable name for the STL.
In order to avoid conceptual confusion, this container is not namaned as 'vector'.
*/

#define TYPE_IRRELATIVE /* type-irrelative */

template <typename T>
class db2DynArray
{
public:
    using value_type = T;

public:
    int32_t length{0}; // length in bytes
    T *data{nullptr};

private:
    int32_t length_mem{0}; // length in bytes

public:
    const int32_t size() const { return this->length / sizeof(T); }
    const int32_t capacity() const { return this->length_mem / sizeof(T); }

public:
    virtual ~db2DynArray()
    {
        if (!this->data)
            return;

        for (int32_t i = 0; i < this->size(); ++i)
            (this->data + i)->~T();

        std::free(this->data);
        this->data = nullptr;
    }

public: // Element access
    auto operator[](const int32_t index) const -> T &
    {
        const auto i = index >= 0 ? index : index + this->size();
        auto ptr = this->data + i;
        return *(T *)(ptr);
        // return *reinterpret_cast<U *>(ptr);
    }

    template <typename U = T>
    auto at(const int32_t &index) const -> U & // If no such element exists, null is returned
    {
        static_assert(sizeof(U) == sizeof(T));

        T *ptr = nullptr;
        if (&index != nullptr)
        {
            auto size = this->size();
            const auto i = index >= 0 ? index : index + size;
            if (0 <= i && i < size)
                ptr = this->data + i;
        }
        return *(U *)(ptr);
    }

public: // Modifiers
    template <typename U = T, typename... Args>
    auto emplace(const int32_t &index, Args &&...args) -> U &
    {
        static_assert(sizeof(U) == sizeof(T));
        assert(0 <= index && index < this->size());

        auto ptr = this->data + index;
        ptr->~U();
        ::new (ptr) U(std::forward<Args>(args)...);

        return *(U *)ptr;
    }

    template <typename U = T, typename... Args>
    auto emplace_back(Args &&...args) -> U &
    {
        static_assert(sizeof(U) == sizeof(T));

        auto size = this->size(); // old size
        this->reserve(size + 1);

        auto ptr = this->data + size;
        // ::new (ptr) U(args...);
        ::new (ptr) U(std::forward<Args>(args)...);
        this->length += sizeof(U);

        return *(U *)ptr;
    }

    template <typename U = T, typename... Args>
    auto append(Args &&...args) -> void
    {
        // (this->emplace_back<U>(std::forward<Args>(args)), ...); // one arg for one U

        static_assert(sizeof(U) == sizeof(T));

        auto size_old = this->size();
        auto size_append = sizeof...(args);
        this->reserve(size_old + size_append);

        auto ptr = this->data + size_old;
        // (::new(ptr++) U(std::forward<Args>(args)), ...); // one arg for one U
        // ::new (ptr) U[size_append](std::forward<Args>(args)...); // all args for every U?
        // ::new (ptr) U[]{std::forward<Args>(args)...}; // new U until all args is used
        new (ptr) U[size_append]{std::forward<Args>(args)...}; // new U by [size_append] even when all args is used
        this->length += sizeof(U) * size_append;
    }

    template <typename U = T>
    auto append_range(const std::initializer_list<U> &arg_list) -> void
    {
        static_assert(sizeof(U) == sizeof(T));

        auto size_old = this->size();
        auto size_append = arg_list.size();
        this->reserve(size_old + size_append);

        auto ptr = this->data + size_old;
        for (const auto &arg : arg_list)
            ::new (ptr++) U(std::move(arg));

        this->length += sizeof(U) * size_append;
    }

    template <typename T_> // for perfect forwarding
    auto push_back(T_ &&t) -> T &
    {
        return this->emplace_back(std::forward<T_>(t));
    }

    auto pop_back() -> void
    {
        (this->data + this->size() - 1)->~T();
        this->length -= sizeof(T);
    }

    /*
    auto resize(const int32_t size) -> void
    {
        auto old_size = this->size();

        if (size == old_size)
            return;
        else if (size < old_size)
        {
            for (int32_t i = size; i < old_size; ++i)
                (this->data + i)->~T();
        }
        else if (size > old_size)
        {
            this->reserve(size);
            for (int32_t i = old_size; i < size; ++i)
                new (this->data + i) T();
        }

        this->length = size * sizeof(T);
    }
    */

public:
    auto for_each(std::function<bool(T &)> func) -> void
    {
        for (int32_t i = 0; i < this->size(); ++i)
            if (!func(this->data[i])) // continue?
                break;
    }

public:
    auto reserve(const int32_t capacity, const bool expand = true) -> void
    {
        auto length_mem = capacity * sizeof(T);
        this->reserve_mem(length_mem, expand);
    }

    TYPE_IRRELATIVE auto reserve_mem(int32_t length_mem, const bool expand = true) -> void
    {
        if (length_mem <= this->length_mem)
            return;

        if (expand)
        {
            auto exp = std::ceil(std::log2(length_mem));
            auto length_mem_exp = std::ceil(std::pow(2, exp));

            assert(length_mem_exp <= INT32_MAX); // 2GB
            assert(length_mem_exp >= length_mem);

            length_mem = length_mem_exp;
        }

        *(void **)(&this->data) = std::realloc(this->data, length_mem);
        this->length_mem = length_mem;
    }
};
