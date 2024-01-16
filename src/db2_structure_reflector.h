#pragma once

#include <cstring> // ::memcpy  std::equal
#include <type_traits>
#include <boost/pfr/core.hpp> // reflect

#include "db2_settings.h"
#include "db2_dynarray.h"

// Implementation of void_t for C++14 and earlier
template <typename...>
using void_t = void;

template <typename T, typename = void>
struct has_value_type : std::false_type
{
};

template <typename T>
struct has_value_type<T, void_t<typename T::value_type>> : std::true_type
{
};

class db2StructReflector
{
    // static
public:
    static db2DynArray<db2StructReflector *> reflectors;

    template <typename T>
    static auto Reflect(const char *type) -> void
    {
        db2StructReflector::reflectors.push(new db2StructReflector());
        db2StructReflector::reflectors[-1]->reflect<T>(type);
    }

    static auto GetReflector(const char *type) -> db2StructReflector *
    {
        for (int i = 0; i < db2StructReflector::reflectors.size(); ++i)
        {
            auto reflector = db2StructReflector::reflectors[i];
            if (std::equal(type, type + 4, reflector->type))
                return reflector;
        }
        return nullptr;
    }

    static auto ClearReflectors() -> void
    {
        for (int i = 0; i < db2StructReflector::reflectors.size(); ++i)
            delete db2StructReflector::reflectors[i];
    }

    template <typename T>
    static constexpr auto Reflectable() -> bool
    {
        if constexpr (has_value_type<T>::value)
            if constexpr (std::is_base_of<db2DynArray<typename T::value_type>, T>::value)
                return true;
        return false;
    }

    // instance
public:
    char type[4]{'N', 'U', 'L', 'L'};
    uint8_t length{0};

    db2DynArray<uint8_t> offsets{};
    db2DynArray<uint8_t> lengths{};

    db2StructReflector *parent{nullptr};
    db2StructReflector *child{nullptr};

    ~db2StructReflector()
    {
        if (this->child)
        {
            this->child->~db2StructReflector();
            delete this->child;
        }
    }

    template <typename T>
    auto reflect(const char *type) -> void
    {
        assert(db2StructReflector::Reflectable<T>());

        ::memcpy(this->type, type, 4);

        using value_type = typename T::value_type;

        this->length = sizeof(value_type);

        if constexpr (db2StructReflector::Reflectable<value_type>())
        {
            this->child = new db2StructReflector();
            child->parent = this;
            child->reflect<value_type>("CHLD");
        }
        else
        {
            static const value_type *const pt{nullptr};
            static const value_type &vaule{*pt};

            boost::pfr::for_each_field(
                vaule,
                [&](auto &field)
                {
                    this->offsets.emplace_back((char *)&field - (char *)&vaule);
                    this->lengths.emplace_back(sizeof(field));
                });
        }
    }
};
