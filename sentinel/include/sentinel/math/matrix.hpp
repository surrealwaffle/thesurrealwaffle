
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>

#include <type_traits>
#include <utility>

#include <sentinel/fundamental_types.hpp>
#include <sentinel/math/vector.hpp>

namespace sentinel { namespace math {

template<class T, std::size_t M, std::size_t N>
struct matrix {
    using vector_type     = vector<T, M>;
    using value_type      = vector<T, M>;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using iterator        = pointer;
    using const_iterator  = const_pointer;

    static_assert(M > 0 && N > 0);
    value_type vectors[N];

    static inline constexpr matrix zero = matrix(/*ZERO INITIALIZED*/);
    static inline constexpr matrix identity = [] {
        auto make_col = [] <std::size_t Column, std::size_t... Row>
                            (std::integral_constant<std::size_t, Column>,
                             std::index_sequence<Row...>) {
            return value_type{static_cast<T>((int)(Column == Row))...};
        };

        return [make_col] <std::size_t... Column> (std::index_sequence<Column...>) {
            return matrix{make_col(std::integral_constant<std::size_t, Column>{},
                                   std::make_index_sequence<M>{})...};
        }(std::make_index_sequence<N>{});
    }();

    constexpr reference       operator[](size_type pos)       { return vectors[pos]; }
    constexpr const_reference operator[](size_type pos) const { return vectors[pos]; }

    constexpr pointer       data() noexcept       { return vectors; }
    constexpr const_pointer data() const noexcept { return vectors; }

    static
    constexpr size_type size() noexcept { return N; }

    constexpr iterator begin() noexcept { return data(); }
    constexpr iterator end()   noexcept { return begin() + size(); }

    constexpr const_iterator cbegin() const noexcept { return data(); }
    constexpr const_iterator cend()   const noexcept { return begin() + size(); }

    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator end()   const noexcept { return cend(); }

    constexpr void fill(const T& value) { for (auto& v : *this) v = value; }

    constexpr bool operator==(const matrix& other) const {
        return std::mismatch(begin(), end(), other.begin()).first == end();
    }

    constexpr bool operator!=(const matrix& other) const {
        return !(*this == other);
    }

    static constexpr matrix filled(const T& value)
    {
        auto make_filled_vec = [&value] (auto) -> vector_type {
            return vector_type::filled(value);
        };
        return [&make_filled_vec] <std::size_t... I> (std::index_sequence<I...>) {
            return matrix{make_filled_vec(I)...};
        } (std::make_index_sequence<N>{});
    }

    template<class BinaryOp>
    constexpr matrix& piecewise_transform(const matrix& other, BinaryOp op)
    {
        auto other_it = other.begin();
        for (auto it = begin(), end = this->end(); it != end; ++it, ++other_it)
            *it = op(*it, *other_it);
        return *this;
    }

    constexpr matrix& operator+=(const matrix& other) { return piecewise_transform(other, std::plus{}); }
    constexpr matrix& operator-=(const matrix& other) { return piecewise_transform(other, std::minus{}); }

    constexpr matrix& operator*=(const T& scalar) { return (*this) *= filled(scalar); }

    template<std::size_t P>
    constexpr auto operator*=(const matrix<T, N, P>& other)
        -> std::enable_if_t<N == P, matrix&> { return (*this = *this * other); }

    constexpr matrix operator+(const matrix& other) const { matrix result = *this; result += other; return result; }
    constexpr matrix operator-(const matrix& other) const { matrix result = *this; result -= other; return result; }

    constexpr matrix operator-() const;
};

struct affine_matrix3d {
    using matrix_type = matrix<real, 3, 3>;
    using vector_type = vector<real, 3>;

    real        scale;
    matrix_type ortho_transform; // SO(3)
    vector_type translation;

    /*
    static inline constexpr affine_matrix3d identity = {1,
                                                        matrix_type::identity,
                                                        vector_type::zero};*/

    constexpr affine_matrix3d& operator*=(const affine_matrix3d& other);

    constexpr affine_matrix3d operator*(const affine_matrix3d& other) const { affine_matrix3d result = *this; result *= other; return result; }
};

template<class T, std::size_t M, std::size_t N>
constexpr matrix<T, N, M> transpose(const matrix<T, M, N>& other)
{
    using size_type = typename matrix<T, N, M>::size_type;
    matrix<T, N, M> result = {};
    for (size_type j = 0; j < M; ++j)     // result column
        for (size_type i = 0; i < N; ++i) // result row
            result[j][i] = other[i][j];
    return result;
}

template<class T, std::size_t M, std::size_t N>
constexpr matrix<T, M, N> matrix<T, M, N>::operator-() const
{
    matrix res = *this;
    for (auto& v : res)
        v = -v;
    return res;
}
template<class T, std::size_t M, std::size_t N>
constexpr matrix<T, M, N> operator*(const T& scalar, const matrix<T, M, N>& mat)
{
    auto result = mat;
    for (auto& v : result)
        v *= scalar;
    return result;
}

template<class T, std::size_t M, std::size_t N, std::size_t P>
constexpr matrix<T, M, P> operator*(const matrix<T, M, N>& a, const matrix<T, N, P>& b)
{
    using matrix_type = matrix<T, M, P>;
    using size_type   = typename matrix_type::size_type;
    matrix<T, M, P> result = matrix<T, M, P>::zero;

    for (size_type i = 0; i < P; ++i)
        for (size_type j = 0; j < N; ++j)
            result.vectors[i] += b.vectors[i][j] * a.vectors[j];

    return result;
}

template<class T, std::size_t M, std::size_t N>
constexpr vector<T, M> operator*(const matrix<T, M, N>& a, const vector<T, N>& b)
{
    using size_type = typename matrix<T, M, N>::size_type;
    vector<T, N> result = vector<T, N>::zero;

    for (size_type i = 0; i < N; ++i)
        result += b[i] * a[i];

    return result;
}

/** \brief Applies an affine transformation onto a vector.
 */
inline constexpr
affine_matrix3d::vector_type operator*(const affine_matrix3d& mat,
                                       const affine_matrix3d::vector_type& vec)
{
    return mat.scale * (mat.ortho_transform * vec) + mat.translation;
}

/** \brief Computes the inverse of an affine transformation.
 */
inline constexpr
affine_matrix3d inverse(const affine_matrix3d& mat)
{
    return {1 / mat.scale, transpose(mat.ortho_transform), -mat.translation};
}

inline constexpr
affine_matrix3d& affine_matrix3d::operator*=(const affine_matrix3d& other)
{
    translation = (*this) * other.translation;
    ortho_transform *= other.ortho_transform;
    scale *= other.scale;
    return *this;
}

} } // namespace sentinel::math
