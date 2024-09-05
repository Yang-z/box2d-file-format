#pragma once

#include <cmath>
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

        auto plain_size = std::max({first.size(), others.size()...});

        db2Tensor result{first.shape, plain_size, false};

        for (auto i = 0; i < plain_size; ++i)
            result[i] = atom_opr(first.at(i), others.at(i)...);

        return result;
    }

    template <typename Atom_Opr>
    static auto Collapse(const Atom_Opr &atom_opr, const db2Tensor &tensor) -> db2Tensor
    {
        if (tensor.shape.size() == 0)
            return tensor;

        assert(tensor.shape[0] != INT32_MAX);

        auto size_of_last_dim = tensor.shape.back();
        assert(size_of_last_dim > 0);

        auto new_shape = tensor.shape;
        new_shape.pop_back();

        // if (size_of_last_dim == 1)
        // {
        //     db2Tensor result = tensor;
        //     result.shape = new_shape;
        //     return result;
        // }

        db2Tensor result{new_shape, false};
        for (auto i = 0; i < result.size(); ++i)
        {
            result[i] = tensor[i * size_of_last_dim];
            for (auto j = 1; j < size_of_last_dim; ++j)
                result[i] = atom_opr(result[i], tensor[i * size_of_last_dim + j]);
        }
        return result;
    }

    template <typename Atom_Opr>
    static auto Collapse_Totally(const Atom_Opr &atom_opr, const db2Tensor &tensor) -> db2Tensor
    {
        db2Tensor result;
        do
            result = db2Tensor::Collapse(atom_opr, tensor);
        while (result.shape.size() != 0);

        return result;
    }

public:
    db2DynArray<uint32_t> shape; // shape of tensor

public: // constructors
    DB2_DYNARRAY_CONSTRUCTORS(db2Tensor, T)

    db2Tensor(const std::initializer_list<uint32_t> &shape_arg_list, const std::initializer_list<T> &arg_list)
        : db2DynArray<T>(arg_list), shape(shape_arg_list) {}

    db2Tensor(const db2DynArray<uint32_t> &shape, const uint32_t plain_size, bool initialize = true)
        : db2DynArray<T>(plain_size, initialize), shape(shape) {}

    db2Tensor(const db2DynArray<uint32_t> &shape, bool initialize = true)
        : shape(shape)
    {
        auto plain_size = 1u;
        for (auto i = 0; i < shape.size(); ++i)
            plain_size *= shape[i];
        this->expand(plain_size, initialize);
    }

    explicit operator bool() const
    {
        return static_cast<bool>(db2Tensor::Collapse_Totally(
            [](T a, T b) -> T
            {
                return static_cast<T>(static_cast<bool>(a) && static_cast<bool>(b));
            },
            *this)[0]);
    }
};

template <typename T = float32_t>
inline auto operator+(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate([](T a, T b) -> T
                                 { return a + b; }, tensor, other);
}

template <typename T = float32_t>
inline auto operator-(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate([](T a, T b) -> T
                                 { return a - b; }, tensor, other);
}

template <typename T = float32_t>
inline auto operator*(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate([](T a, T b) -> T
                                 { return a * b; }, tensor, other);
}

template <typename T = float32_t>
inline auto operator/(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate([](T a, T b) -> T
                                 { return a / b; }, tensor, other);
}

template <typename T = float32_t>
inline auto operator%(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    if constexpr (std::is_integral_v<T>)
        return Operate([](T a, T b) -> T
                       { return a % b; }, tensor, other);
    else
        return db2Tensor<T>::Operate(static_cast<T (*)(T, T)>(std::fmod), tensor, other);
}

template <typename T = float32_t>
inline auto sin(const db2Tensor<T> &tensor) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::sin), tensor);
}

template <typename T = float32_t>
inline auto cos(const db2Tensor<T> &tensor) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::cos), tensor);
}

template <typename T = float32_t>
inline auto tan(const db2Tensor<T> &tensor) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::tan), tensor);
}

template <typename T = float32_t>
inline auto asin(const db2Tensor<T> &tensor) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::asin), tensor);
}

template <typename T = float32_t>
inline auto acos(const db2Tensor<T> &tensor) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::acos), tensor);
}

template <typename T = float32_t>
inline auto atan(const db2Tensor<T> &tensor) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::atan), tensor);
}

template <typename T = float32_t>
inline auto pow(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T, T)>(std::pow), tensor, other);
}

template <typename T = float32_t>
inline auto log(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::log), tensor) / db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::log), other);
}

template <typename T = float32_t>
inline auto abs(const db2Tensor<T> &tensor) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::abs), tensor);
}

template <typename T = float32_t>
inline auto ceil(const db2Tensor<T> &tensor) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::ceil), tensor);
}

template <typename T = float32_t>
inline auto round(const db2Tensor<T> &tensor) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::round), tensor);
}

template <typename T = float32_t>
inline auto floor(const db2Tensor<T> &tensor) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate(static_cast<T (*)(T)>(std::floor), tensor);
}

template <typename T = float32_t>
inline auto equal(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate([](T a, T b) -> T
                                 { return static_cast<T>(a == b); }, tensor, other);
}

template <typename T = float32_t>
inline auto greater(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate([](T a, T b) -> T
                                 { return static_cast<T>(a > b); }, tensor, other);
}

template <typename T = float32_t>
inline auto greater_equal(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate([](T a, T b) -> T
                                 { return static_cast<T>(a >= b); }, tensor, other);
}

template <typename T = float32_t>
inline auto less(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate([](T a, T b) -> T
                                 { return static_cast<T>(a < b); }, tensor, other);
}

template <typename T = float32_t>
inline auto less_equal(const db2Tensor<T> &tensor, const db2Tensor<T> &other) -> db2Tensor<T>
{
    return db2Tensor<T>::Operate([](T a, T b) -> T
                                 { return static_cast<T>(a <= b); }, tensor, other);
}