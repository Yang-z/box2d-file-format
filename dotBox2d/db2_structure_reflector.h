#pragma once

#include <cstring>            // ::memcpy  std::equal
#include <boost/pfr/core.hpp> // reflect

#include "db2_settings.h"
#include "db2_container.h"

class db2StructReflector
{
    // static
public:
    static db2DynArray<db2StructReflector> reflectors;

    template <typename T>
    static auto reflect(const char *type) -> void
    {
        db2StructReflector::reflectors.emplace_back(type).reflect<T>();
    }

    static auto getReflector(const char *type) -> db2StructReflector *
    {
        for (int i = 0; i < db2StructReflector::reflectors.size(); ++i)
        {
            auto &reflector = db2StructReflector::reflectors[i];
            if (std::equal(type, type + 4, reflector.type))
                return &reflector;
        }
        return nullptr;
    }

    // instance
public:
    char type[4]{'N', 'U', 'L', 'L'};
    uint8_t length{0};

    db2DynArray<uint8_t> offsets{};
    db2DynArray<uint8_t> lengths{};

    db2StructReflector(const char *type)
    {
        ::memcpy(this->type, type, 4);
    }

    template <typename T>
    auto reflect() -> void
    {
        this->length = sizeof(T);

        static T *pt{nullptr};
        static T &t{*pt};

        boost::pfr::for_each_field(
            t,
            [&](auto &field)
            {
                this->offsets.emplace_back((char *)&field - (char *)&t);
                this->lengths.emplace_back(sizeof(field));
            });
    }
};
