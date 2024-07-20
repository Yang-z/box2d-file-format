#pragma once

#include <cstring> // std::memcpy std::memset
#include <cassert> // assert static_assert
#include <cstdlib> // std::malloc std::free std::realloc
#include <cmath>   // std::log2 std::pow

#include <type_traits> // std::enable_if
#include <functional>

#include "common/db2_settings.h"
#include "common/db2_nullval.h"

/*
It's a std::vector-like container.
Vector is a concept in maths or physics, and it's not a suitable name for the STL.
In order to avoid conceptual confusion, this container is not namaned as 'vector'.

Know issue: when capacity increases, memory addresses of existing data could be changed.
So, any referencing to the original data could become invalid. Only index accessasing is
guaranteed to be safe.
*/

#define DB2_CONSTRUCTORS(CLS, ...)                                       \
    CLS() = default;                                                     \
    CLS(const CLS<__VA_ARGS__> &other) = delete;                         \
    CLS<__VA_ARGS__> &operator=(const CLS<__VA_ARGS__> &other) = delete; \
    CLS(CLS<__VA_ARGS__> &&other) { *this = std::move(other); };         \
    CLS<__VA_ARGS__> &operator=(CLS<__VA_ARGS__> &&other) { return std::memcpy(this, &other, sizeof(*this)), std::memset(&other, 0, sizeof(*this)), *this; };

template <typename T>
class db2DynArray
{
public:
    using value_type = T;

public:
    T *data{nullptr};
    int32_t length{0}; // length in bytes
private:
    int32_t length_mem{0}; // length in bytes

public:
    const int32_t size() const { return this->length / sizeof(T); }
    const int32_t capacity() const { return this->length_mem / sizeof(T); }

public: // Constructors
    DB2_CONSTRUCTORS(db2DynArray, T);
    virtual ~db2DynArray() { this->clear(); }

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

        if (index != nullval)
        {
            auto size = this->size();
            const auto i = index >= 0 ? index : index + size;
            if (0 <= i && i < size)
                return *(U*)(this->data + i);
        }
        return nullval;
    }

public: // Modifiers
    template <typename U = T, typename... Args>
    auto emplace(const int32_t index, Args &&...args) -> U &
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
        // ::new (ptr) U[size_append](std::forward<Args>(args)...); // one arg for one U, but error "could not convert ... 'int' to 'db2DictElement'..."
        // ::new (ptr) U[]{std::forward<Args>(args)...}; // new U until all args is used
        ::new (ptr) U[size_append]{std::forward<Args>(args)...}; // new U by [size_append] even when all args is used
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
            ::new (ptr++) U(std::move(arg)); // forced move?

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

    auto clear() -> void
    {
        if (!this->data)
            return;

        for (int32_t i = 0; i < this->size(); ++i)
            (this->data + i)->~T();

        std::free(this->data);
        this->data = nullptr;
        this->length = 0;
        this->length_mem = 0;
    }

public:
    auto find_index(const std::function<bool(T &)> &func) -> int32_t
    {
        for (int32_t i = 0; i < this->size(); ++i)
            if (func(this->data[i]))
                return i;
        return INT32_MIN;
    }

    auto find(const std::function<bool(T &)> &func) -> T &
    {
        int32_t index = this->find_index(func);
        return index == INT32_MIN ? nullval : this->data[index];
    }

    auto for_each(std::function<bool(T &)> func) -> void
    {
        for (int32_t i = 0; i < this->size(); ++i)
            if (!func(this->data[i])) // continue?
                break;
    }

public:
    auto reserve(const int32_t capacity, const bool expand = true) -> void { this->reserve_mem(capacity * sizeof(T), expand); }

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

template <typename T, typename T_pfx = void>
class db2DynArrayWithPrefix : public db2DynArray<T>
{

public:
    using prefix_type = T_pfx;

public:
    T_pfx *prefix{nullptr};
    int32_t length_pfx{0};

public:
    DB2_CONSTRUCTORS(db2DynArrayWithPrefix, T, T_pfx)
    virtual ~db2DynArrayWithPrefix() { this->clear_pfx(); }

public:
    auto clear_pfx() -> void
    {
        if constexpr (!std::is_void_v<T_pfx>)
        {
            if (this->prefix)
            {
                this->prefix->~T_pfx();
                std::free(this->prefix);
                this->prefix = nullptr;
                this->length_pfx = 0;
            }
        }
    }

public:
    template <typename... Args>
    auto emplace_pfx(Args &&...args) -> default_ref_t<T_pfx>
    {
        if constexpr (!std::is_void_v<T_pfx>)
        {
            this->reserve_pfx_mem(sizeof(T_pfx));
            // this->prefix->~T_pfx();
            ::new (this->prefix) T_pfx(std::forward<Args>(args)...);
            this->length_pfx = sizeof(T_pfx);
            return *this->prefix;
        }
    }

    TYPE_IRRELATIVE auto reserve_pfx_mem(int32_t length_pfx) -> void
    {
        if (this->prefix)
            return;
        *(void **)(&this->prefix) = std::malloc(length_pfx);
    }
};