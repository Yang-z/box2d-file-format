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
    int size{0};
    int capacity{0};
    char tag[4]{'N', 'U', 'L', 'L'};
    T *data{nullptr};

public:
    db2Container(const char *tag = nullptr)
    {
        if (tag)
            ::memcpy(this->tag, tag, 4);
    }

    ~db2Container()
    {
        if (!this->data)
            return;

        for (int i = 0; i++; i < this->size)
            (this->data + i)->~T();

        free(this->data);
        this->data = nullptr;
    }

    auto operator[](int index) -> T &
    {
        this->nomalizeIndex(index);
        return this->data[index];
    }

    auto reserve(int capacity, bool expand = true) -> void
    {
        if (capacity <= this->capacity)
            return;

        if (expand)
            capacity = this->expandCapacity(capacity);

        this->capacity = capacity;
        auto data = this->data;
        this->data = (T *)malloc(this->capacity * sizeof(T));

        if (!data)
            return;

        ::memcpy(this->data, data, this->size * sizeof(T));
        ::free(data);
    }

    auto resize(const int &size) -> void
    {
        if (size == this->size)
            return;
        else if (size < this->size)
        {
            for (int i = size; i < this->size; i++)
                (this->data + i)->~T();
        }
        else if (size > this->size)
        {
            this->reserve(size);
            for (int i = this->size; i < size; i++)
                new (this->data + i) T();
        }
        this->size = size;
    }

    auto read(std::ifstream &fs, const int &length) -> void
    {
        assert(length % sizeof(T) == 0);
        auto size = length / sizeof(T);

        this->reserve(this->size + size, false);

        auto begin = this->data + this->size;
        fs.read((char *)(begin), size * sizeof(T));
        this->size += size;
    }

    auto write(std::ofstream &fs) -> void
    {
        auto length = this->size * sizeof(T);
        fs.write((char *)this->data, length);
    }

    auto push(const T &t) -> void
    {
        this->reserve(this->size + 1);
        ::new (this->data + this->size) T(t);
        ++this->size;
    }

    auto pop() -> void
    {
        (this->data + --this->size)->~T();
    }

    template <typename... Args>
    auto emplace_back(Args &&...args) -> T &
    {
        // this->reserve(this->size++ + 1); // wrong
        this->reserve(this->size + 1);
        ++this->size;

        ::new (this->data + this->size - 1) T(args...);
        return this->data[this->size - 1];
    }

private:
    auto expandCapacity(const int &size) -> int
    {
        auto exp = int(std::log2(size));
        auto capacity = int(std::pow(2, exp + 1));
        return capacity;
    }

    inline auto nomalizeIndex(int &index) -> void
    {
        if (index < 0)
            index += this->size;
        assert(index >= 0 && index < this->size);
    }
};