#pragma once

#include <algorithm> // std::max({...})

#include "containers/db2_dynarray.h"

/*

*/

// template <typename T, typename U>
// concept db2_same_as = std::is_same_v<U, T>;

// template <typename Atom_Opr, typename T, typename... Args>
// concept AtomOperator =
//     // requires(Atom_Opr f, T t, Args... args) {
//     //     { f(t, args...) } -> std::convertible_to<T>;
//     // };

//     // (... && std::same_as<T, Args>)
//     // && std::invocable<Atom_Opr, T, Args...>
//     // && std::same_as<std::invoke_result_t<Atom_Opr, T, Args...>, T>;

template <typename T = float32_t>
class db2Tensor : public db2DynArray<T>
{

public: // static
    // T (*atom_opr)(T, Ts...) // only works for function pointer
    // std::function<T(T, Ts...)> atom_opr // need warp callable by std::function before passing
    template <typename Atom_Opr, typename... db2Tensors>
        requires(... && std::same_as<db2Tensor, db2Tensors>)
    static auto Operate(const Atom_Opr &atom_opr, const db2Tensor &first, const db2Tensors &...others) -> db2Tensor
    {
        if (!(true && ... && (others.shape == first.shape)))
            return {};

        assert(first.is_valid());

        auto loose_plain_size = std::max({first.size(), others.size()...});

        db2Tensor result{};
        result.init(first.shape, loose_plain_size, false);

        for (auto i = 0u; i < loose_plain_size; ++i)
            result[i] = atom_opr(first.at(i), others.at(i)...);

        return result;
    }

    template <typename Atom_Opr>
    static auto Collapse(const Atom_Opr &atom_opr, const db2Tensor &tensor) -> db2Tensor
    {
        if (tensor.shape.size() == 0)
            return tensor;

        assert(tensor.is_valid());

        auto size_of_last_dim = tensor.shape.back();
        auto old_plain_size = tensor.size();

        if (size_of_last_dim == UINT32_MAX) // last collapsing dimension is loose
            size_of_last_dim = old_plain_size;

        uint32_t new_plain_size = old_plain_size / size_of_last_dim;
        if (old_plain_size % size_of_last_dim != 0)
            ++new_plain_size;

        auto new_shape = tensor.shape;
        new_shape.pop_back();

        db2Tensor result{};
        result.init(new_shape, new_plain_size, false);
        for (auto i = 0; i < new_plain_size; ++i)
        {
            result[i] = tensor.at(i * size_of_last_dim);
            for (auto j = 1; j < size_of_last_dim; ++j)
                result[i] = atom_opr(result[i], tensor.at(i * size_of_last_dim + j));
        }
        return result;
    }

    template <typename Atom_Opr>
    static auto Collapse_Totally(const Atom_Opr &atom_opr, const db2Tensor &tensor) -> db2Tensor
    {
        db2Tensor result{};
        do
            result = db2Tensor::Collapse(atom_opr, tensor);
        while (result.shape.size() != 0);

        return result;
    }

public: // properties
    //
    db2DynArray<uint32_t> shape; // shape of tensor

    auto plain_size() const -> uint32_t
    {
        auto p_size = 1u;
        for (auto i = 0; i < this->shape.size(); ++i)
            if (shape[i] != UINT32_MAX)
                p_size *= shape[i];
        // else // loose
        //     return this->size();
        return p_size;
    }

    auto is_valid() const -> bool
    {
        for (auto i = 0; i < this->shape.size(); ++i)
            if ((i != 0 && shape[i] == UINT32_MAX) || shape[i] == 0)
                return false;
        return true;
    }

    auto is_overflow() const -> bool
    {
        assert(this->is_valid());

        if (this->size() > this->plain_size() && this->shape.at(0) != UINT32_MAX)
            return true;
        return false;
    }

public: // constructors
    db2Tensor() = default;

    // for scalar
    db2Tensor(const T scalar)
        : db2DynArray<T>{scalar} {}

    // for vector
    db2Tensor(const std::initializer_list<T> &arg_list)
        : db2DynArray<T>(arg_list), shape{static_cast<uint32_t>(arg_list.size())} {}

    db2Tensor(const std::initializer_list<T> &arg_list, const std::initializer_list<uint32_t> &shape_arg_list)
        : db2DynArray<T>(arg_list), shape(shape_arg_list) {}

    auto init(const db2DynArray<uint32_t> &shape, const uint32_t loose_plain_size = 0, bool initialize = true) -> void
    {
        this->shape = shape;
        this->db2DynArray<T>::init(loose_plain_size == 0 ? this->plain_size() : loose_plain_size, initialize);
    }

    explicit operator bool() const
    {
        return db2Tensor::Collapse_Totally([](T a, T b) -> T
                                           { return a && b; }, *this)[0];
    }
};
