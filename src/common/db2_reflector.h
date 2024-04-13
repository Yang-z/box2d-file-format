#pragma once

#include <cctype>             //std::toupper std::tolower
#include <algorithm>          // std::equal
#include <type_traits>        // std::is_same ...
#include <typeinfo>           // std::typeinfo
#include <boost/pfr/core.hpp> // reflect

#include "db2_settings.h"
#include "containers/db2_dynarray.h"

class db2Reflector
{
public:
    struct pack_info
    {
        uint8_t length{0};
        db2DynArray<uint8_t> offsets{};
        db2DynArray<uint8_t> lengths{};
    };

public: // static
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

    template <typename T>
    static auto Reflect_POD(pack_info *pack) -> void
    {
        pack->length = sizeof(T);

        // T shoud be of a POD (plain old data) type

        static const T *const pt{nullptr};
        static const T &vaule{*pt};

        boost::pfr::for_each_field(
            vaule,
            [&](auto &field)
            {
                pack->offsets.push_back((char *)&field - (char *)&vaule);
                pack->lengths.push_back(sizeof(field));
            } //
        );
    }

public: // instance
    char type[4]{0, 0, 0, 0};
    char type_link[4]{0, 0, 0, 0};

    size_t id;
    // std::type_info info;

    pack_info *prefix{nullptr};
    pack_info *value{nullptr};

    db2Reflector *parent{nullptr};
    db2Reflector *child{nullptr};

    ~db2Reflector()
    {
        if (this->prefix)
            delete this->prefix, this->prefix = nullptr;

        if (this->value)
            delete this->value, this->value = nullptr;

        if (this->child)
            delete this->child, this->child = nullptr;
    }

    template <typename CK_T>
    auto reflect(const char *type) -> void
    {
        std::memcpy(this->type, type, 4);
        std::memcpy(this->type_link, type, 4);
        this->id = typeid(CK_T).hash_code();

        if constexpr (!has_value_type_v<CK_T>)
            Reflect_POD<CK_T>((this->value = new pack_info(), this->value));

        else
        {
            // asume CK_T has prefix_type if it has value_type
            if constexpr (!std::is_void_v<typename CK_T::prefix_type>)
                Reflect_POD<CK_T::prefix_type>((this->prefix = new pack_info(), this->prefix));

            using value_type = typename CK_T::value_type;
            this->type[2] = !has_value_type_v<value_type> ? std::toupper(this->type[2]) : std::tolower(this->type[2]);
            this->type[3] = this->parent ? 0 : this->type[3];

            this->type_link[0] = std::tolower(this->type_link[0]);

            if constexpr (!has_value_type_v<value_type>)
                Reflect_POD<value_type>((this->value = new pack_info(), this->value));
            else
            {
                this->child = new db2Reflector();
                child->parent = this;
                child->reflect<value_type>(type);
            }
        }
    }
};
