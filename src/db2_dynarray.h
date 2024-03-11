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
    int32_t length_men{0}; // length in bytes

public:
    const int32_t size() const { return this->length / sizeof(T); }
    const int32_t capacity() const { return this->length_men / sizeof(T); }

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

        U *ptr = nullptr;
        if (&index != nullptr)
        {
            const auto i = index >= 0 ? index : index + this->size();
            if (0 <= i < this->size())
                ptr = this->data + i;
        }
        return *(U *)(ptr);
    }

public: // Modifiers


    template <typename U = T, typename... Args>
    auto emplace(const int32_t &index, Args &&...args) -> U &
    {
        static_assert(sizeof(U) == sizeof(T));
        assert(0 <= index < this->size());

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
        auto length_men = capacity * sizeof(T);
        this->reserve_men(length_men, expand);
    }

    TYPE_IRRELATIVE auto reserve_men(int32_t length_men, const bool expand = true) -> void
    {
        if (length_men <= this->length_men)
            return;

        if (expand)
        {
            auto exp = int32_t(std::log2(length_men));
            length_men = int32_t(std::pow(2, exp + 1));
        }

        assert(length_men <= INT32_MAX); // 2GB

        *(void **)(&this->data) = std::realloc(this->data, length_men);
        this->length_men = length_men;
    }
};
