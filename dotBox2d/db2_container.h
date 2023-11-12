#pragma once

// #include <stdlib.h>
#include <assert.h>
#include <math.h>

/*
It's a std::vector-like container.
Vector is a concept in maths or physics, and it's not a suitable name for the STL.
In order to avoid conceptual confusion, this container is not namaned as 'vector'.
*/

template <typename T>
class db2DynArray
{
public:
    int32_t length{0};
    T *data{nullptr};

private:
    int32_t length_men{0};

public:
    ~db2DynArray()
    {
        if (!this->data)
            return;

        for (int i = 0; i < this->size(); ++i)
            (this->data + i)->~T();

        ::free(this->data);
        this->data = nullptr;
    }

    auto operator[](const int index) -> T &
    {
        const auto i = index >= 0 ? index : index + this->size();
        return this->data[i];
    }

    auto size() -> const int
    {
        return this->length / sizeof(T);
    }

    auto capacity() -> int
    {
        return this->length_men / sizeof(T);
    }

    auto reserve(const int capacity, const bool expand = true) -> void
    {
        auto length_men = capacity * sizeof(T);
        this->reserve_men(length_men, expand);
    }

    auto resize(const int size) -> void
    {
        auto old_size = this->size();

        if (size == old_size)
            return;
        else if (size < old_size)
        {
            for (int i = size; i < old_size; i++)
                (this->data + i)->~T();
        }
        else if (size > old_size)
        {
            this->reserve(size);
            for (int i = old_size; i < size; i++)
                new (this->data + i) T();
        }

        this->length = size * sizeof(T);
    }

    auto push(const T &t) -> void
    {
        this->reserve(this->size() + 1);
        ::new (this->data + this->size()) T(t);
        this->length += sizeof(T);
    }

    auto pop() -> void
    {
        (this->data + this->size() - 1)->~T();
        this->length -= sizeof(T);
    }

    template <typename... Args>
    auto emplace_back(Args &&...args) -> T &
    {
        auto size = this->size(); // old size

        this->reserve(size + 1);
        this->length += sizeof(T); // size() already changed

        ::new (this->data + size) T(args...);
        return this->data[size];
    }

public:
    /* type-irrelative */
    auto reserve_men(int length_men, const bool expand = true) -> void
    {
        if (length_men <= this->length_men)
            return;

        if (expand)
        {
            auto exp = int(std::log2(length_men));
            length_men = int(std::pow(2, exp + 1));
        }

        *(void **)(&this->data) = ::realloc(this->data, length_men);
        this->length_men = length_men;
    }
};