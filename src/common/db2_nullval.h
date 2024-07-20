#pragma once

class db2Nullval
{
public:
    static auto Instance() -> db2Nullval &;

private:
    char data[1]{};
    db2Nullval();

public:
    template <typename T>
    operator T &()
    {
        return reinterpret_cast<T &>(*this);
    }

};

template <typename T>
bool operator==(const T &t, const db2Nullval &null)
{
    return &t == (void *)&null;
}

template <typename T>
bool operator!=(const T &t, const db2Nullval &null)
{
    return &t != (void *)&null;
}

#define nullval (db2Nullval::Instance())
