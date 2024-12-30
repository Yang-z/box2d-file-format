#pragma once

#include <cctype>             //std::toupper std::tolower
#include <algorithm>          // std::equal
#include <type_traits>        // std::is_same ...
#include <typeinfo>           // std::typeinfo
#include <boost/pfr/core.hpp> // reflect

#include "db2_settings.h"
#include "containers/db2_dynarray.h"

// #include "stdio.h"

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
    // A single linked library or executable file keeps only one copy of an inline static data menber,
    // but it can still cause multiple instances across different libraries.
    // So, don't use inline static data menbers and forget about "header-only".
    static db2DynArray<db2Reflector *> reflectors;

    // // function-local static object in an inline function is also duplicated accross different libraries.
    // static int caller;
    // inline static auto LocalStaticTester() -> int
    // {
    //     static int i{0};
    //     ++i;
    //     printf("i = %d\n", i);
    //     return i;
    // }

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

    template <typename CK_T>
    static auto Reflect(const char *type) -> void
    {
        db2Reflector::reflectors.push_back(new db2Reflector())->reflect<CK_T>(type);
    }

    static auto ClearReflectors() -> void
    {
        for (int i = 0; i < db2Reflector::reflectors.size(); ++i)
            delete db2Reflector::reflectors[i];
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
        static db2Reflector *Reflector = nullptr;
        if (Reflector)
            return Reflector;

        for (int i = 0; i < db2Reflector::reflectors.size(); ++i)
        {
            auto &reflector = db2Reflector::reflectors[i];
            if (reflector->info == &typeid(CK_T))
                return Reflector = reflector, Reflector;
        }
        return nullptr;
    }

    template <typename CK_T>
    static auto Is(const char *type) -> bool
    {
        return std::equal(type, type + 4, db2Reflector::GetReflector<CK_T>()->type);
    }

    template <typename CK_T>
    static auto IsRefOf(const char *type) -> bool
    {
        static auto reflector = db2Reflector::GetReflector<CK_T>();
        return std::equal(type, type + 4, reflector->type_ref);
    }

public: // instance
    alignas(4) char type[4]{0, 0, 0, 0};
    DB2_DEPRECATED char type_ref[4]{0, 0, 0, 0};

    bool is_type_int = false;        // int type is allowed for sub-chunks
    bool is_dynamic_nesting = false; // dynamic nesting is allowed for sub-chunks
    const db2DynArray<int32_t> *end_types = nullptr;

    const std::type_info *info;

    pack_info *prefix{nullptr};
    pack_info *value{nullptr};

    db2Reflector *parent{nullptr};

private:
    db2Reflector *child{nullptr};

public:
    db2Reflector *get_child(const char *type)
    {
        if (type && this->is_dynamic_nesting)
            if (!this->end_types->has(*reinterpret_cast<const int32_t *>(type)))
                return this;

        return this->child;
    }

public:
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
        DB2_DEPRECATED std::memcpy(this->type_ref, type, 4);
        this->info = &typeid(CK_T);

        // type_type
        if constexpr (has_type_type_v<CK_T>)
            if constexpr (std::is_same_v<typename CK_T::type_type, int32_t>)
            {
                assert(this->parent != nullptr);
                this->is_type_int = true;
            }

        // dynamic nesting
        if constexpr (has_flag_dynamic_nesting_v<CK_T>)
        {
            assert(this->parent != nullptr);
            this->is_dynamic_nesting = true;
            this->end_types = &CK_T::end_types;
        }

        // prefix
        if constexpr (has_prefix_type_v<CK_T>)
            if constexpr (!std::is_void_v<typename CK_T::prefix_type>)
            {
                this->prefix = new pack_info();
                Reflect_POD<CK_T::prefix_type>(this->prefix);
            }

        // value
        if constexpr (!has_value_type_v<CK_T>)
        {
            this->value = new pack_info();
            Reflect_POD<CK_T>(this->value);
        }
        else
        {
            using value_type = typename CK_T::value_type;
            this->type[2] = !has_value_type_v<value_type> ? std::toupper(this->type[2]) : std::tolower(this->type[2]);
            // this->type[3] = this->parent ? 0 : this->type[3];

            // ref type
            DB2_DEPRECATED // this->type_ref[0] = this->type_ref[0] - 64;

                if constexpr (!has_value_type_v<value_type>)
            {
                this->value = new pack_info();
                Reflect_POD<value_type>(this->value);
            }
            else
            {
                this->child = new db2Reflector();
                child->parent = this;
                child->reflect<value_type>(type);
            }
        }
    }
};
