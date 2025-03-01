#pragma once

#include "db2_chunk.h"

/*
CSON（C/C++ Structured Object Notation) is a JSON-like but binary data format.

Know issue: Due to the memory shifting issue brought by the base db2DynArray, referencing
of chunk data could become invalid after new data is added to the same chunk. When editting
CSON, referencing to the chunk data should be cautious. Use index or key to access the data
when nessary.
*/

DB2_PRAGMA_PACK_ON

struct db2DictElement
{
    int32_t key{0};

    // the type of value, could be used to identify which chunk the value links to.
    char type0{0}, type1{0}, type2{0}, type3{0};

    uint32_t value{UINT32_MAX};

} DB2_NOTE(sizeof(db2DictElement) == 12);
static_assert(std::is_trivially_copyable_v<db2DictElement>);

struct db2Dict : public db2Chunk<db2DictElement>
{

public:
    template <typename CK_T>
    auto find(const int32_t &key) -> db2DictElement &
    {
        static auto *reflector = db2Reflector::GetReflector<CK_T>();
        static char *type = reflector ? reflector->type_ref : nullptr;

        return this->find(key, type);
    }

    auto find(const int32_t &key, const char *type = nullptr) -> db2DictElement &
    {
        return this->db2DynArray<db2DictElement>::find(
            [&](db2DictElement &element) -> bool
            {
                bool match_key = key == nullval || element.key == key;
                bool match_type = type == nullptr || std::equal(&element.type0, &element.type0 + 4, type);
                return match_key && match_type;
            });
    }

public: // Element access
    template <typename CK_T = uint32_t>
    /* or at_ref */
    auto ref(const int32_t &key) -> uint32_t & // if no such element exists, null is returned
    {
        auto &element = this->find<CK_T>(key); // could be null
        return element != nullval ? element.value : nullval;
    }

    template <typename CK_T = uint32_t, typename vv_type = default_value_t<CK_T>>
    auto at(const int32_t &key) -> vv_type & // if no such element exists, null is returned
    {
        auto &element = this->find<CK_T>(key);                                  // could be null
        return element != nullval ? this->dereference<CK_T>(element) : nullval; // could be null
    }

    template <typename CK_T = uint32_t, typename vv_type = default_value_t<CK_T>>
    auto dereference(db2DictElement &element) -> vv_type & // could be null
    {
        // element could not be null now

        this->handle_type<CK_T>(element); // necessary?

        if constexpr (has_value_type_v<CK_T>)
            return this->root->get<CK_T>().at(element.value); // could be null
        else
            return reinterpret_cast<vv_type &>(element.value); // not null
    }

public: // Modifiers
    template <typename CK_T>
    auto handle_type(db2DictElement &element, bool set = false) -> void
    {
        static_assert(has_value_type_v<CK_T> || sizeof(CK_T) == sizeof(int32_t));

        static auto *reflector = db2Reflector::GetReflector<CK_T>();
        if (!reflector)
            return;

        if (set)
            std::memcpy(&element.type0, reflector->type_ref, 4);
        else
            assert(std::equal(&element.type0, &element.type0 + 4, reflector->type));
    }

    template <typename CK_T = uint32_t>
    /* or get_ref (the dict element's value which stores the index of a linked value or stores a 32-bit POD) */
    auto link(const int32_t &key, const uint32_t &v_index = nullval) -> uint32_t & // performing an insertion if such key does not already exist
    {
        auto &element = this->emplace_ref<CK_T>(key, v_index);
        return element.value;
    }

    template <typename CK_T = uint32_t, typename vv_type = default_value_t<CK_T>>
    /* or get_value (the linked value) */
    auto get(const int32_t &key) -> vv_type & // performing an insertion if such key does not already exist
    {
        auto &element = this->emplace_ref<CK_T>(key);      // not null
        auto &vv_value = this->dereference<CK_T>(element); // could be null
        return vv_value != nullval ? vv_value : this->emplace_val<CK_T>(element);
    }

    template <typename CK_T = uint32_t>
    /* or get_element */
    auto emplace_ref(const int32_t &key, const uint32_t &v_index = nullval) -> db2DictElement &
    {
        auto p_element = &this->find<CK_T>(key);
        if (*p_element == nullval)
        {
            p_element = &this->db2DynArray<db2DictElement>::emplace_back(key); // Aggregate Initialization
            this->handle_type<CK_T>(*p_element, true);
        }
        if (v_index != nullval)
            p_element->value = v_index;
        return *p_element;
    }

    template <typename CK_T = uint32_t, typename vv_type = default_value_t<CK_T>, typename... Args>
    auto emplace_val(db2DictElement &element, Args &&...args) -> vv_type &
    {
        assert(element != nullval);
        auto &v_index = element.value;

        if constexpr (has_value_type_v<CK_T>)
        {
            auto &chunk = this->root->get<CK_T>();
            if (0 <= v_index && v_index < chunk.size())
                return chunk.emplace(v_index, std::forward<Args>(args)...);
            else
                return v_index = chunk.size(), chunk.emplace_back(std::forward<Args>(args)...);
        }
        else
        {
            // static_assert(sizeof(vv_type) == sizeof(int32_t));
            ::new (&v_index) vv_type(std::forward<Args>(args)...);
            return reinterpret_cast<vv_type &>(v_index);
        }
    }

    template <typename CK_T = uint32_t, typename vv_type = default_value_t<CK_T>, typename... Args>
    auto emplace(const int32_t &key, Args &&...args) -> vv_type &
    {
        auto &element = this->emplace_ref<CK_T>(key);
        return this->emplace_val<CK_T>(element, std::forward<Args>(args)...);
    }
};

struct db2List : public db2Chunk<uint32_t>
{

    // auto init() -> void {}
    // template <typename... Args>
    // auto init(Args &&...args) -> void { this->append(std::forward<Args>(args)...); }

public: // Element access
    template <typename CK_T = value_type>
    /* or at_ref */
    auto ref(const uint32_t index) -> value_type & // could be null
    {
        this->handle_type<CK_T>();
        return this->db2DynArray<value_type>::at(index); // could be null
    }

    template <typename CK_T = value_type, typename vv_type = default_value_t<CK_T>>
    auto at(const uint32_t index) -> vv_type & // if no such element exists, null is returned
    {
        auto &v_index = this->db2DynArray<value_type>::at(index); // could be null
        return v_index != nullval ? this->dereference<CK_T>(v_index) : nullval;
    }

    template <typename CK_T = value_type, typename vv_type = default_value_t<CK_T>>
    auto dereference(const value_type &v_index) -> vv_type & // could be null
    {
        // v_index could not be null

        this->handle_type<CK_T>();
        if constexpr (has_value_type_v<CK_T>)
            return this->root->get<CK_T>().at(v_index); // could be null
        else
            return reinterpret_cast<vv_type &>(v_index); // could be null
    }

public: // Modifiers
    template <typename CK_T>
    auto handle_type(bool set = false) -> void
    {
        if constexpr (!has_value_type_v<CK_T>)
            static_assert(sizeof(CK_T) == sizeof(value_type));

        static auto *reflector = db2Reflector::GetReflector<CK_T>();
        if (!reflector)
            return;

        if (this->size() != 0)
            return assert(std::equal(this->type, this->type + 4, reflector->type));

        if (set)
            // Type field is used to store the type of element, and it is settled when the first element is added.
            std::memcpy(this->type, reflector->type, 4);
    }

    template <typename CK_T = value_type>
    auto emplace_back_ref(const value_type &v_index = UINT32_MAX) -> value_type &
    {
        this->handle_type<CK_T>(true);
        return this->db2DynArray<value_type>::emplace_back(v_index);
    }

    template <typename CK_T = value_type, typename vv_type = default_value_t<CK_T>, typename... Args>
    auto emplace_back(Args &&...args) -> vv_type &
    {
        this->handle_type<CK_T>(true);
        if constexpr (has_value_type_v<CK_T>)
        {
            auto &chunk = this->root->get<CK_T>();
            this->db2DynArray<value_type>::emplace_back(chunk.size());
            return chunk.emplace_back(std::forward<Args>(args)...);
        }
        else
        {
            return this->db2DynArray<value_type>::emplace_back<vv_type>(std::forward<Args>(args)...);
        }
    }

    // template <typename CK_T = value_type>
    // auto append_range_ref(const std::initializer_list<int32_t> &arg_list) -> void
    // {
    //     this->set_type<CK_T>();
    //     this->db2Chunk<value_type>::append_range(arg_list);
    // }
};

struct db2String : public db2Chunk<char>
{
    // db2String(const char *str) { this->append(str); }
    auto init(const char *str) -> void { this->append_range(str); }

    auto c_str() -> char * { return this->data; }

    auto operator=(const char *str) -> void { this->clear(), this->append_range(str); }
    auto operator+=(const char *str) -> void { this->append_range(str); }

    auto append_range(const char *str) -> void
    {
        if (!str)
            return;

        if (this->data && this->length > 0 && this->data[this->length - 1] == 0)
            --this->length;

        auto len_str = std::strlen(str);
        auto len_new = this->length + len_str + 1; // +1 for '\0'
        this->reserve(len_new);
        std::memcpy(this->data + this->length, str, len_str + 1);
        // std::strcpy(this->data + this->length, str);
        this->length = len_new;
    }
};

DB2_PRAGMA_PACK_OFF

using CKDict = db2Chunk<db2Dict>;
using CKList = db2Chunk<db2List>;
using CKString = db2Chunk<db2String>;

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
