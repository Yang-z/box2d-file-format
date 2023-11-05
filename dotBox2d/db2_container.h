#pragma once

#include <fstream>
#include <cstring>

#include <assert.h>
#include <math.h>

/*
It's a std::vector-like container.
Vector is a concept in maths or physics, and it's not a suitable name for the STL.
In order to avoid conceptual confusion, this container is not namaned as 'vector'.
*/

template <typename T>
class db2Container
{
public:
    int32_t length{0};
    char type[4]{'N', 'U', 'L', 'L'};
    T *data{nullptr};
    int32_t CRC{0};

private:
    int32_t length_men{0};

public:
    db2Container(const char *type = nullptr)
    {
        if (type)
            ::memcpy(this->type, type, 4);
    }

    ~db2Container()
    {
        if (!this->data)
            return;

        for (int i = 0; i++; i < this->size())
            (this->data + i)->~T();

        free(this->data);
        this->data = nullptr;
    }

    auto operator[](int index) -> T &
    {
        this->nomalizeIndex(index);
        return this->data[index];
    }

    auto size() -> int
    {
        return this->length / sizeof(T);
    }

    auto capacity() -> int
    {
        return this->length_men / sizeof(T);
    }

    auto reserve_men(int length_men, bool expand = true) -> void
    {
        if (length_men <= this->length_men)
            return;

        if (expand)
        {
            auto exp = int(std::log2(length_men));
            length_men = int(std::pow(2, exp + 1));
        }

        auto data_old = this->data;
        auto length_men_old = this->length_men;

        *(void **)(&this->data) = malloc(length_men);
        this->length_men = length_men;

        if (!data_old)
            return;

        ::memcpy(this->data, data_old, length_men_old);
        ::free(data_old);
    }

    auto reserve(int capacity, bool expand = true) -> void
    {
        auto length_men = capacity * sizeof(T);
        this->reserve_men(length_men, expand);
    }

    auto resize(const int &size) -> void
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

    auto read(std::ifstream &fs, const int &length) -> void
    {
        // assert(length % sizeof(T) == 0);

        this->reserve_men(this->length + length, false);
        auto begin = (char *)this->data + this->length;
        fs.read(begin, length);
        this->length += length;
    }

    auto write(std::ofstream &fs) -> void
    {
        fs.write((char *)this->data, this->length);
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
        this->reserve(this->size() + 1);
        this->length += sizeof(T);

        ::new (this->data + this->size() - 1) T(args...);
        return this->data[this->size() - 1];
    }

private:
    inline auto nomalizeIndex(int &index) -> void
    {
        if (index < 0)
            index += this->size();
        assert(index >= 0 && index < this->size());
    }
};