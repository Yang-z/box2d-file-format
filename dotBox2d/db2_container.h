#pragma once

#include <fstream>
#include <cstring>

#include <assert.h>
#include <math.h>

template <typename T>
class db2Vector
{
public:
    int size{0};
    int capacity{0};
    char tag[4]{'N', 'U', 'L', 'L'};
    T *data{nullptr};

public:
    db2Vector(const char *tag = nullptr)
    {
        if (tag)
            ::memcpy(this->tag, tag, 4);
    }

    ~db2Vector()
    {
        if (!this->data)
            return;

        for (int i = 0; i++; i < this->size)
            (this->data + i)->~T();

        free(this->data);
        this->data = nullptr;
    }

    auto operator[](const int &i) -> T &
    {
        return this->data[i];
    }

    auto reserve(int capacity, bool expand = false) -> void
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

    auto read(std::ifstream &fs, const int &length) -> void
    {
        assert(length % sizeof(T) == 0);
        auto size = length / sizeof(T);

        this->reserve(this->size + size);

        auto begin = this->data + this->size;
        fs.read((char *)(begin), size * sizeof(T));
        this->size += size;
    }

    auto write(std::ofstream &fs) -> void
    {
        auto length = this->size * sizeof(T);
        fs.write((char *)this->data, length);
    }

    auto push() -> T &
    {
        this->reserve(this->size + 1, true);
        ::new (this->data + this->size) T();
        return this->data[this->size++];
    }

    auto pop() -> void
    {
        (this->data + --this->size)->~T();
    }

private:
    auto expandCapacity(const int &size) -> int
    {
        auto exp = int(std::log2(size));
        auto capacity = int(std::pow(2, exp + 1));
        return capacity;
    }
};