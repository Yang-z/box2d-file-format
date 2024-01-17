#pragma once

#include <cstring> // ::memcpy  std::equal
#include <type_traits>
#include <typeinfo>
#include <boost/pfr/core.hpp> // reflect

#include "db2_settings.h"
#include "db2_dynarray.h"

// Implementation of void_t for C++14 and earlier
template <typename...>
using void_t = void;

template <typename CK_T, typename = void>
struct has_value_type : std::false_type
{
};

template <typename CK_T>
struct has_value_type<CK_T, void_t<typename CK_T::value_type>> : std::true_type
{
};

class db2Reflector
{
    // static
public:
    static db2DynArray<db2Reflector *> reflectors;

    template <typename CK_T>
    static auto Reflect(const char *type) -> void
    {
        db2Reflector::reflectors.push(new db2Reflector())->reflect<CK_T>(type);
    }

    static auto GetReflector(const char *type) -> db2Reflector *
    {
        for (int i = 0; i < db2Reflector::reflectors.size(); ++i)
        {
            auto & reflector = db2Reflector::reflectors[i];
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
            auto & reflector = db2Reflector::reflectors[i];
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

    template <typename CK_T>
    static constexpr auto Reflectable() -> bool
    {
        if constexpr (has_value_type<CK_T>::value)
            if constexpr (std::is_base_of<db2DynArray<typename CK_T::value_type>, CK_T>::value)
                return true;
        return false;
    }

    // instance
public:
    char type[4]{'N', 'U', 'L', 'L'};
    size_t id;
    // std::type_info info;
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
        }
    }

    template <typename CK_T>
    auto reflect(const char *type) -> void
    {
        assert(db2Reflector::Reflectable<CK_T>());

        ::memcpy(this->type, type, 4);
        this->id = typeid(CK_T).hash_code();

        using value_type = typename CK_T::value_type;

        this->length = sizeof(value_type);

        if constexpr (db2Reflector::Reflectable<value_type>())
        {
            this->child = new db2Reflector();
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
                    this->offsets.push((char *)&field - (char *)&vaule);
                    this->lengths.push(sizeof(field));
                });
        }
    }
};
