#pragma once

#include "db2_chunk.h"

/*
CSON（C/C++ Structured Object Notation) is a JSON-like binary data format.
*/

DB2_PRAGMA_PACK_ON

struct db2DictElement
{
    int32_t key{0};

    // the type of value, could be used to identify which chunk the value links to.
    char type0, type1, type2, type3;

    int32_t value{-1};

} DB2_NOTE(sizeof(db2DictElement) == 12);

struct db2Dict : public db2Chunk<db2DictElement>
{
    // auto find(const int32_t key) -> db2DictElement *
    // {
    //     db2DictElement *match = nullptr;
    //     this->for_each(
    //         [&](auto &element)
    //         {
    //             if (element.key == key)
    //             {
    //                 match = &element;
    //                 return false; // continue?
    //             }
    //             return true;
    //         } //
    //     );
    //     return match;
    // }

    template <typename CK_T>
    auto find(const int32_t &key) -> db2DictElement &
    {
        auto *reflector = db2Reflector::GetReflector<CK_T>();
        const char *type = reflector ? reflector->type : nullptr;
        return this->find(key, type);
    }

    auto find(const int32_t &key, const char *type = nullptr) -> db2DictElement &
    {
        for (int32_t i = 0; i < this->size(); ++i)
        {
            auto &element = this->data[i];

            bool match_key = &key == nullptr || element.key == key;
            bool match_type = type == nullptr || std::equal(&(element.type0), &(element.type0) + 4, type);

            if (match_key && match_type)
                return element;
        }
        return *(db2DictElement *)nullptr;
    }

    template <typename CK_T = int32_t>
    auto ref(const int32_t &key, const int32_t &v_index = *(int32_t *)nullptr) -> int32_t &
    {
        auto &element = this->emplace_ref<CK_T>(key, v_index);
        return element.value;
    }

    template <typename CK_T = int32_t, typename vv_type = typename default_value_type<CK_T>::type>
    auto at(const int32_t &key) -> vv_type & // If no such element exists, null is returned
    {
        auto &element = this->find<CK_T>(key);
        return this->dereference<CK_T>(element);
    }

    template <typename CK_T = int32_t, typename vv_type = typename default_value_type<CK_T>::type>
    auto get(const int32_t &key) -> vv_type & // performing an insertion if such key does not already exist
    {
        auto &element = this->emplace_ref<CK_T>(key);
        auto &vv_value = this->dereference<CK_T>(element);
        if (&vv_value) // or ???
            return vv_value;
        return this->emplace_val<CK_T>(element.value);
    }

    template <typename CK_T = int32_t, typename vv_type = typename default_value_type<CK_T>::type>
    auto dereference(db2DictElement &element) -> vv_type &
    {
        if (&element == nullptr)
            return *(vv_type *)nullptr;

        static auto *reflector = db2Reflector::GetReflector<CK_T>();
        assert(std::equal(&(element.type0), &(element.type0) + 4, reflector->type));

        if constexpr (!has_value_type<CK_T>::value)
            return reinterpret_cast<vv_type &>(element.value);
        else
            return this->root->get<CK_T>().at(element.value);
    }

    template <typename CK_T = int32_t>
    auto emplace_ref(const int32_t &key, const int32_t &v_index = *(int32_t *)nullptr) -> db2DictElement &
    {
        auto p_element = &this->find<CK_T>(key);
        if (p_element == nullptr)
        {
            p_element = &this->db2Chunk<db2DictElement>::emplace_back();
            p_element->key = key;
            static auto *reflector = db2Reflector::GetReflector<CK_T>();
            if (reflector)
                std::memcpy(&p_element->type0, reflector->type, 4);
        }
        if (&v_index)
            p_element->value = v_index;
        return *p_element;
    }

    template <typename CK_T = int32_t, typename vv_type = typename default_value_type<CK_T>::type, typename... Args>
    auto emplace_val(int32_t &value, Args &&...args) -> vv_type &
    {
        assert(&value);

        if constexpr (!has_value_type<CK_T>::value)
        {
            static_assert(sizeof(vv_type) == sizeof(value));
            ::new (&value) vv_type(std::forward<Args>(args)...);
            return reinterpret_cast<vv_type &>(value);
        }
        else
        {
            auto &chunk = this->root->get<CK_T>();
            if (0 <= value && value < chunk.size())
            {
                return chunk.emplace(value, std::forward<Args>(args)...);
            }
            else
            {
                value = chunk.size();
                return chunk.emplace_back(std::forward<Args>(args)...);
            }
        }
    }

    template <typename CK_T = int32_t, typename vv_type = typename default_value_type<CK_T>::type, typename... Args>
    auto emplace(const int32_t &key, Args &&...args) -> vv_type &
    {
        auto &value = this->emplace_ref<CK_T>(key).value;
        return this->emplace_val<CK_T>(value, std::forward<Args>(args)...);
    }
};

struct db2List : public db2Chunk<int32_t>
{

    // auto init() -> void {}
    // template <typename... Args>
    // auto init(Args &&...args) -> void { this->append(std::forward<Args>(args)...); }

    template <typename CK_T = int32_t>
    auto ref(const int32_t &index) -> int32_t &
    {
        static auto *reflector = db2Reflector::GetReflector<CK_T>();
        assert(std::equal(&this->type, &this->type + 4, reflector->type));

        return this->db2Chunk<int32_t>::at(index);
    }

    template <typename CK_T = int32_t, typename vv_type = typename default_value_type<CK_T>::type>
    auto at(const int32_t &index) -> vv_type & // If no such element exists, null is returned
    {
        auto &v_index = this->db2Chunk<int32_t>::at(index);
        return this->dereference<CK_T>(v_index);
    }

    template <typename CK_T = int32_t, typename vv_type = typename default_value_type<CK_T>::type>
    auto dereference(const int32_t &v_index) -> vv_type &
    {
        static auto *reflector = db2Reflector::GetReflector<CK_T>();
        assert(std::equal(&this->type, &this->type + 4, reflector->type));

        if constexpr (!has_value_type<CK_T>::value)
            return reinterpret_cast<vv_type &>(v_index);
        else
            return this->root->get<CK_T>().at(v_index);
    }

    template <typename CK_T>
    auto set_type() -> void
    {
        static auto *reflector = db2Reflector::GetReflector<CK_T>();
        if (reflector)
            if (this->size() == 0)
                // Type field is used to store the type of element, and it is settled when the first element is added.
                std::memcpy(this->type, reflector->type, 4);
            else
                assert(std::equal(this->type, this->type + 4, reflector->type));
    }

    template <typename CK_T = int32_t>
    auto emplace_back_ref(const int32_t &v_index = *(int32_t *)nullptr) -> int32_t &
    {
        this->set_type<CK_T>();

        auto &value = this->db2Chunk<int32_t>::emplace_back();
        if (&v_index)
            value = v_index;
        return value;
    }

    template <typename CK_T = int32_t, typename vv_type = typename default_value_type<CK_T>::type, typename... Args>
    auto emplace_back(Args &&...args) -> vv_type &
    {
        auto &index = this->emplace_back_ref<CK_T>();

        if constexpr (!has_value_type<CK_T>::value)
        {
            static_assert(sizeof(vv_type) == sizeof(index));
            ::new (&index) vv_type(std::forward<Args>(args)...);
            return reinterpret_cast<vv_type &>(index);
        }
        else
        {
            auto &chunk = this->root->get<CK_T>();
            index = chunk.size();
            return chunk.emplace_back(std::forward<Args>(args)...);
        }
    }

    // template <typename CK_T = int32_t>
    // auto append_range_ref(const std::initializer_list<int32_t> &arg_list) -> void
    // {
    //     this->set_type<CK_T>();
    //     this->db2Chunk<int32_t>::append_range(arg_list);
    // }
};

struct db2String : public db2Chunk<char>
{
    // db2String(const char *str) { this->append(str); }
    auto init(const char *str) -> void { this->append_range(str); }

    auto c_str() -> char * { return this->data; }

    auto operator+=(const char *str) -> void { this->append_range(str); }

    auto append_range(const char *str) -> void
    {
        if (!str)
            return;

        if (this->data && this->length > 0 && this->data[this->length - 1] == 0)
            --this->length;

        auto len_str = std::strlen(str);
        auto len_new = this->length + len_str + 1;
        this->reserve(len_new);
        std::memcpy(this->data + this->length, str, len_str + 1);
        // std::strcpy(this->data + this->length, str);
        this->length = len_new;
    }
};

DB2_PRAGMA_PACK_OFF

struct db2ChunkType_CSON
{
    static constexpr const char PODI[4]{'P', 'O', 'D', 'I'};
    static constexpr const char PODF[4]{'P', 'O', 'D', 'F'};

    static constexpr const char DIcT[4]{'D', 'I', 'c', 'T'};
    static constexpr const char LIsT[4]{'L', 'I', 's', 'T'};
    static constexpr const char STrG[4]{'S', 'T', 'r', 'G'};

    // static constexpr const char dIcT[4]{'d', 'I', 'c', 'T'};
    // static constexpr const char lIsT[4]{'l', 'I', 's', 'T'};
    // static constexpr const char sTrG[4]{'s', 'T', 'r', 'G'};

    static bool IsRegistered;
    static bool RegisterType();

} DB2_NOTE(sizeof(db2ChunkType));
