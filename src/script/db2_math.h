#pragma once

#include <cmath>

#include "common/db2_settings.h"
#include "db2_tensor.h"

class db2Math
{
public:
    static auto Function(const int32_t type, const std::initializer_list<db2Tensor<float32_t>> &tensor_list) -> std::function<void()>;
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
        return db2Tensor<T>::Operate([](T a, T b) -> T
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