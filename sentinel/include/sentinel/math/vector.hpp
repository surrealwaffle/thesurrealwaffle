
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>
#include <cmath>

#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>

#include <sentinel/config.hpp>


#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/register/point.hpp>

namespace sentinel { namespace math {

template<class T, std::size_t N>
struct vector {
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using iterator        = pointer;
    using const_iterator  = const_pointer;

    static_assert(N > 0);
    value_type array[N];

    static inline constexpr vector zero = vector(/*ZERO INITIALIZED*/);

    constexpr reference       operator[](size_type pos)       { return array[pos]; }
    constexpr const_reference operator[](size_type pos) const { return array[pos]; }

    constexpr pointer       data() noexcept       { return array; }
    constexpr const_pointer data() const noexcept { return array; }

    static
    constexpr size_type size() noexcept { return N; }

    constexpr iterator begin() noexcept { return array; }
    constexpr iterator end()   noexcept { return begin() + size(); }

    constexpr const_iterator cbegin() const noexcept { return array; }
    constexpr const_iterator cend()   const noexcept { return begin() + size(); }

    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator end()   const noexcept { return cend(); }

    constexpr void fill(const T& value) { for (auto& v : *this) v = value; }

    constexpr bool operator==(const vector& other) const {
        return std::mismatch(begin(), end(), other.begin()).first == end();
    }

    constexpr bool operator!=(const vector& other) const {
        return !(*this == other);
    }

    static constexpr vector filled(const T& value)
    {
        auto return_fill_value = [&value] (auto&&...) -> const T& { return value; };
        return [&return_fill_value] <std::size_t... I> (std::index_sequence<I...>) {
            return vector{return_fill_value(I)...};
        } (std::make_index_sequence<N>{});
    }

    template<class BinaryOp>
    constexpr vector& pointwise_transform(const vector& other, BinaryOp op)
    {
        auto other_it = other.begin();
        for (auto it = begin(), end = this->end(); it != end; ++it, ++other_it)
             *it = op(*it, *other_it);
        return *this;
    }

    constexpr vector& operator+=(const vector& other) { return pointwise_transform(other, std::plus{}); }
    constexpr vector& operator-=(const vector& other) { return pointwise_transform(other, std::minus{}); }
    constexpr vector& operator*=(const vector& other) { return pointwise_transform(other, std::multiplies{}); }
    constexpr vector& operator/=(const vector& other) { return pointwise_transform(other, std::divides{}); }

    constexpr vector& operator*=(const T& scalar) { return (*this) *= filled(scalar); }

    constexpr vector operator+(const vector& other) const { vector result = *this; result += other; return result; }
    constexpr vector operator-(const vector& other) const { vector result = *this; result -= other; return result; }
    constexpr vector operator*(const vector& other) const { vector result = *this; result *= other; return result; }
    constexpr vector operator/(const vector& other) const { vector result = *this; result /= other; return result; }

    constexpr vector operator-() const;
};

template<class T, std::size_t N>
constexpr vector<T, N> vector<T, N>::operator-() const
{
    vector res = *this;
    for (auto& v : res)
        v = -v;
    return res;
}

/** \brief Computes the dot product of two vectors.
 */
template<class T, std::size_t N>
constexpr T dot(const vector<T, N>& a, const vector<T, N>& b)
{
    T result = static_cast<T>(0);
    for (std::size_t pos = 0; pos < N; ++pos)
        result += a[pos] * b[pos];
    return result;
}

/** \brief Computes the cross product of two vector.
 */
template<class T>
constexpr vector<T, 3> cross(const vector<T, 3>& a, const vector<T, 3>& b)
{
    return {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

/** \brief Computes the square of the norm of a vector.
 */
template<class T, std::size_t N>
constexpr float norm2(const vector<T, N>& vec) { return dot(vec, vec); }

/** \brief Computes the norm of a vector.
 */
template<class T, std::size_t N>
constexpr float norm(const vector<T, N>& vec) { return std::sqrt(norm2(vec)); }

/** \brief Computes a normalized vector.
 *
 * If the norm of \a vec is less than or equal to SENTINEL_VECTOR_SMALL_NORM,
 * the returned vector is the `0` vector.
 */
template<class T, std::size_t N>
constexpr vector<T, N> normalized(const vector<T, N>& vec)
{
    const auto length = norm(vec);
    return length > SENTINEL_VECTOR_SMALL_NORM ? (1 / length) * vec
                                               : vector<T, N>::zero;
}

/** \brief Performs scalar-vector multiplication.
 */
template<class T, std::size_t N>
constexpr vector<T, N> operator*(const T& scalar, const vector<T, N>& vec)
{
    return vector<T, N>::filled(scalar) * vec;
}

/** \brief Performs scalar-vector multiplication.
 */
template<class T, std::size_t N>
constexpr vector<T, N> operator*(const vector<T, N>& vec, const T& scalar)
{
    return vec * vector<T, N>::filled(scalar);
}

template<class T, std::size_t N, class BinaryOp>
constexpr vector<T, N> pointwise_compose(const vector<T, N>& a,
                                         const vector<T, N>& b,
                                         BinaryOp op)
{
    vector<T, N> result = a;
    a.pointwise_accumulate(a, std::cref(op));
    return result;
}

using fvec2 = vector<float, 2>;
using fvec3 = vector<float, 3>;

using dvec2 = vector<double, 2>;
using dvec3 = vector<double, 3>;

} } // namespace sentinel::math

BOOST_GEOMETRY_REGISTER_POINT_2D(sentinel::math::fvec2,
                                 float,
                                 boost::geometry::cs::cartesian,
                                 operator[](0),
                                 operator[](1)
                                 );

BOOST_GEOMETRY_REGISTER_POINT_3D(sentinel::math::fvec3,
                                 float,
                                 boost::geometry::cs::cartesian,
                                 operator[](0),
                                 operator[](1),
                                 operator[](2)
                                 );

BOOST_GEOMETRY_REGISTER_POINT_2D(sentinel::math::dvec2,
                                 double,
                                 boost::geometry::cs::cartesian,
                                 operator[](0),
                                 operator[](1)
                                 );

BOOST_GEOMETRY_REGISTER_POINT_3D(sentinel::math::dvec3,
                                 double,
                                 boost::geometry::cs::cartesian,
                                 operator[](0),
                                 operator[](1),
                                 operator[](2)
                                 );
