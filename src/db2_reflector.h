#pragma once

#include <cctype>             //std::toupper std::tolower
#include <algorithm>          // std::equal
#include <type_traits>        // std::is_same ...
#include <typeinfo>           // std::typeinfo
#include <boost/pfr/core.hpp> // reflect

#include "db2_settings.h"
#include "db2_dynarray.h"

// Implementation of void_t for C++14 and earlier
template <typename...>
using void_t = void;

// Implementation of has_value_type
template <typename CK_T, typename = void>
struct has_value_type : std::false_type
{
};

template <typename CK_T>
struct has_value_type<CK_T, void_t<typename CK_T::value_type>> : std::true_type
{
};

// Implementation of default_value_type
template <typename T, typename = void>
struct default_value_type
{
    using type = T;
};

template <typename T>
struct default_value_type<T, void_t<typename T::value_type>>
{
    using type = typename T::value_type;
};

class db2Reflector
{
    // static
public:
    static db2DynArray<db2Reflector *> reflectors;

    template <typename CK_T>
    static auto Reflect(const char *type) -> void
    {
        db2Reflector::reflectors.push_back(new db2Reflector())->reflect<CK_T>(type);
    }

    static auto GetReflector(const char *type) -> db2Reflector *
    {
        for (int i = 0; i < db2Reflector::reflectors.size(); ++i)
        {
            auto &reflector = db2Reflector::reflectors[i];
            if (std::equal(type, type + 4, reflector->type))
                return reflector;
        }
        return nullptr;
    }

    template <typename CK_T>
    static auto GetReflector() -> db2Reflector *
    {
        for (int i = 0; i < db2Reflector::reflectors.size(); ++i)
        {
            auto &reflector = db2Reflector::reflectors[i];
            if (reflector->id == typeid(CK_T).hash_code())
                return reflector;
        }
        return nullptr;
    }

    static auto ClearReflectors() -> void
    {
        for (int i = 0; i < db2Reflector::reflectors.size(); ++i)
            delete db2Reflector::reflectors[i];
    }

    // instance
public:
    char type[4]{'N', 'U', 'L', 'L'};
    size_t id;
    // std::type_info info;

    // element paremeters
    uint8_t length{0};
    db2DynArray<uint8_t> offsets{};
    db2DynArray<uint8_t> lengths{};

    db2Reflector *parent{nullptr};
    db2Reflector *child{nullptr};

    ~db2Reflector()
    {
        if (this->child)
        {
            this->child->~db2Reflector();
            delete this->child;
            this->child = nullptr;
        }
    }

    template <typename CK_T>
    auto reflect(const char *type) -> void
    {
        std::memcpy(this->type, type, 4);
        if (this->parent)
            this->type[3] = 0;

        this->id = typeid(CK_T).hash_code();

        using value_type = typename default_value_type<CK_T>::type;

        // this->type[2] = !has_value_type<value_type>::value ? std::toupper(this->type[2]) : std::tolower(this->type[2]);

        if constexpr (!has_value_type<value_type>::value)
        {
            this->type[2] = std::toupper(this->type[2]);

            this->reflect_POD<value_type>();
        }
        else
        {
            this->type[2] = std::tolower(this->type[2]);

            this->child = new db2Reflector();
            child->parent = this;
            child->reflect<value_type>(type);
        }
    }

    template <typename T>
    auto reflect_POD() -> void
    {
        this->length = sizeof(T);

        // T shoud be of a POD (plain old data) type

        static const T *const pt{nullptr};
        static const T &vaule{*pt};

        boost::pfr::for_each_field(
            vaule,
            [&](auto &field)
            {
                this->offsets.push_back((char *)&field - (char *)&vaule);
                this->lengths.push_back(sizeof(field));
            } //
        );
    }
};
