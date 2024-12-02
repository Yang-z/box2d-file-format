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

#define DB2_DYNARRAY_CONSTRUCTORS(CLS)                                                                     \
    CLS() = default;                                                                                       \
    CLS(const CLS &other) { this->copy(other); }                                                           \
    CLS(CLS &&other) { this->move(std::move(other)); }                                                     \
    CLS(const std::initializer_list<typename CLS::value_type> &arg_list) { this->append_range(arg_list); } \
    virtual ~CLS() { this->clear(); }                                                                      \
    CLS &operator=(const CLS &other) { return this->copy(other), *this; }                                  \
    CLS &operator=(CLS &&other) { return this->move(std::move(other)), *this; }                            \
    bool operator==(const CLS &other) const { return this->equal(other); }                                 \
    bool operator!=(const CLS &other) const { return !this->equal(other); }

template <typename T>
class db2DynArray
{
public:
    using value_type = T;

public:
    T *data{nullptr};
    uint32_t length{0}; // length in bytes
protected:
    uint32_t length_mem{0}; // length in bytes

public:
    const uint32_t size() const { return this->length / sizeof(T); }
    const uint32_t capacity() const { return this->length_mem / sizeof(T); }

public: // constructors and initiators
    DB2_DYNARRAY_CONSTRUCTORS(db2DynArray)

    virtual auto init(uint32_t size, bool initialize = true) -> void { this->expand(size, initialize); }

    auto copy(const db2DynArray &other) -> void
    {
        this->clear();
        this->reserve_mem(other.length, false);
        std::memcpy(this->data, other.data, other.length);
        this->length = other.length;
    };

    auto move(db2DynArray &&other) -> void
    {
        this->clear();
        std::memcpy(this, &other, sizeof(*this));
        std::memset(&other, 0, sizeof(*this));
    };

    auto equal(const db2DynArray &other) const -> bool
    {
        return this->length == other.length &&
               std::memcmp(this->data, other.data, this->length) == 0;
    };

public: // Element access
    /**/
    auto operator[](const uint32_t index) const -> T & // no bounds checking
    {
        return *(T *)(this->data + index);
        // return *reinterpret_cast<U *>(this->data + index);
    }

    template <typename U = T>
    auto at(const uint32_t index) const -> U & // If no such element exists, null is returned
    {
        static_assert(sizeof(U) == sizeof(T));

        // if this is nullval, the size should be 0
        // UINT32_MAX is excluded naturally
        if (0 <= index && index < this->size())
            return *(U *)(this->data + index);
        return nullval;
    }

    auto at_defalut(const uint32_t index, const T &default_ = nullval) const -> T &
    {
        auto &value = this->at(index);
        return value == nullval ? default_ : value;
        // Temporaries live until the end of the full-expression in which they were created.
        // If you pass a temporary to default_, you should copy the return value rather than take a reference.
        // If you bind it to a const reference, lifetime of temporary passed to default_ will be extended??
    }

    auto front() const -> T & { return this->at(0); }
    auto back() const -> T & { return this->at(this->size() - 1); }

public: // Modifiers
    /**/
    template <typename U = T>
    auto get(const uint32_t index) -> U & // performing an extension if index is out of bounds. nessary?
    {
        static_assert(sizeof(U) == sizeof(T));

        if (index != UINT32_MAX)
        {
            this->expand(index + 1);
            return *(U *)(this->data + index);
        }
        return nullval;
    }

    template <typename U = T, typename... Args>
    auto emplace(const int32_t index, Args &&...args) -> U & // should be within bounds
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
        ::new (ptr) U(std::forward<Args>(args)...);
        this->length += sizeof(U);

        return *(U *)ptr;
    }

    template <typename U = T, typename... Args>
    auto insert(const int32_t index, Args &&...args) -> U &
    {
        static_assert(sizeof(U) == sizeof(T));

        auto size = this->size(); // old size
        this->reserve(size + 1);

        auto ptr = this->data + size;
        ::memmove(ptr + 1, ptr, (size - index) * sizeof(U));
        ::new (ptr) U(std::forward<Args>(args)...);
        this->length += sizeof(U);

        return *(U *)ptr;
    }

    template <typename U = T, typename... Args>
    auto erase(const int32_t index) -> U &
    {
        static_assert(sizeof(U) == sizeof(T));

        auto size = this->size(); // old size

        auto ptr = this->data + index;
        ptr->~U();
        ::memmove(ptr, ptr + 1, (size - index - 1) * sizeof(U));
        this->length -= sizeof(U);

        return *(U *)ptr;
    }

    /*
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
    */

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

    auto expand(const uint32_t size, bool initialize = true) -> void
    {
        auto old_size = this->size();
        if (size > old_size)
        {
            this->reserve(size);
            if (initialize)
                for (uint32_t i = old_size; i < size; ++i)
                    ::new (this->data + i) T();
            this->length = size * sizeof(T);
        }
    }

    auto shrink(const uint32_t size) -> void
    {
        auto old_size = this->size();
        if (size < old_size)
        {
            for (uint32_t i = size; i < old_size; ++i)
                (this->data + i)->~T();
            this->length = size * sizeof(T);
        }
    }

    auto resize(const uint32_t size) -> void
    {
        this->expand(size);
        this->shrink(size);
    }

    auto virtual clear() -> void
    {
        if (!this->data)
            return; // assume length == 0

        for (int32_t i = 0; i < this->size(); ++i)
            (this->data + i)->~T();

        std::free(this->data);
        this->data = nullptr;
        this->length = 0;
        this->length_mem = 0;
    }

public:
    auto find_index(const std::function<bool(T &)> &func) const -> uint32_t
    {
        // if this is nullval the size should be 0
        for (uint32_t i = 0; i < this->size(); ++i)
            if (func(this->data[i]))
                return i;
        return UINT32_MAX;
    }

    auto find(const std::function<bool(T &)> &func) const -> T &
    {
        uint32_t index = this->find_index(func);
        return index == UINT32_MAX ? nullval : this->data[index];
    }

    auto has(const T &t) const -> bool
    {
        uint32_t index = this->find_index([&t](T &t_)
                                          { return t_ == t; });
        return index != UINT32_MAX;
    }

    auto for_each(std::function<bool(T &)> func) -> void
    {
        for (uint32_t i = 0; i < this->size(); ++i)
            if (!func(this->data[i])) // continue?
                break;
    }

public:
    auto reserve(const uint32_t capacity, const bool exp = true) -> void { this->reserve_mem(capacity * sizeof(T), exp); }

    TYPE_IRRELATIVE auto reserve_mem(uint32_t length_mem, const bool exp = true) -> void
    {
        if (length_mem <= this->length_mem)
            return;

        if (exp)
        {
            auto exp = std::ceil(std::log2(length_mem));
            auto length_mem_exp = std::ceil(std::pow(2, exp));

            assert(length_mem_exp <= UINT32_MAX); // 4GB
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
    uint32_t length_pfx{0};

public: // constructors and initiators
    DB2_DYNARRAY_CONSTRUCTORS(db2DynArrayWithPrefix)

    auto copy(const db2DynArrayWithPrefix &other) -> void
    {
        this->db2DynArray<T>::copy(other);

        this->clear_pfx();
        this->reserve_pfx_mem(other.length_pfx);
        std::memcpy(this->prefix, other.prefix, other.length_pfx);
        this->length_pfx = other.length_pfx;
    }

    auto move(db2DynArrayWithPrefix &&other) -> void
    {
        this->clear();
        std::memcpy(this, &other, sizeof(*this));
        std::memset(&other, 0, sizeof(*this));
    }

    auto equal(const db2DynArrayWithPrefix &other) const -> bool
    {
        return static_cast<db2DynArray<T> &>(*this) == static_cast<db2DynArray<T> &>(other) &&
               this->length_pfx == other.length_pfx &&
               std::memcmp(this->prefix, other.prefix, this->length_pfx) == 0;
    }

public:
    auto virtual clear_pfx() -> void
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

    auto virtual clear() -> void
    {
        this->clear_pfx();
        this->db2DynArray<T>::clear();
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

    TYPE_IRRELATIVE auto reserve_pfx_mem(uint32_t length_pfx) -> void
    {
        if (this->prefix)
            return;
        *(void **)(&this->prefix) = std::malloc(length_pfx);
    }
};