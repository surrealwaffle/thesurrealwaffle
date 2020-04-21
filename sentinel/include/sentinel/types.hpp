
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>
#include <utility>

#include <sentinel/config.hpp>

#include <sentinel/math/vector.hpp>
#include <sentinel/math/matrix.hpp>
#include <sentinel/fundamental_types.hpp>

namespace sentinel {

/** \addtogroup types
 * @{
 */

// ------------------------
// Vector/Math types

using real2d = math::vector<real, 2>;
using real3d = math::vector<real, 3>;
using real4d = math::vector<real, 4>;

using matrix2d        = math::matrix<real, 2, 2>;
using matrix3d        = math::matrix<real, 3, 3>;
using affine_matrix3d = math::affine_matrix3d;

static_assert(sizeof(real2d) == 8);
static_assert(sizeof(real3d) == 12);
static_assert(sizeof(real4d) == 16);

static_assert(sizeof(matrix3d) == 0x24);
static_assert(sizeof(affine_matrix3d) == 0x34);

// ------------------------
// Structured types

using real2 = real[2];
using real3 = real[3];
using real4 = real[4];

using argbf = real4;

using point2d = real2d;
using point3d = real3d;

using position2d = real2d;
using position3d = real3d;

using direction2d = real2d;
using direction3d = real3d;

struct real_bounds {
    real min;
    real max;
};

struct short_bounds {
    int16 min;
    int16 max;
};

inline constexpr real
lerp(const real f, real_bounds const& bounds) noexcept {
    return f * (bounds.max - bounds.min) + bounds.min;
}

template<std::size_t N>
inline constexpr signature
make_signature(char const (&s)[N]) noexcept {
    auto get = [&s] (std::size_t i) -> std::int32_t { return i < N ? s[i] : ' '; };
    return get(3u)
         | (get(2u) << 8)
         | (get(1u) << 16)
         | (get(0u) << 24);
}

// --------------------------------
// Traits

template<class T>
struct is_tag : std::false_type { };

template<class T>
inline constexpr bool is_tag_v = is_tag<T>::value;

/** \brief Assist the dereferencing facility provided by identities.
 *
 * This template is specialized for each supported datum type.
 * Each specialization must supply at least a static member function `array` which
 * can be invoked as detailed below.
 *
 * When dereferencing the identity, the returned value is by the expression
 * `identity_traits::array()[id.index()]`, where `id` is the identity being dereferenced.
 *
 * If the static member function `morph` is present, the returned value is by the
 * expression `identity_traits::morph(identity_traits::array()[id.index()])`.
 *
 * \a DatumType for specializations must not be cv-qualified.
 */
template<class DatumType>
struct identity_traits;

template<class T, class = std::void_t<>>
struct has_array_member : std::false_type { };

template<class T>
struct has_array_member<T, std::void_t<decltype(T::array)>> : std::true_type { };

template<class T, class = std::void_t<>>
struct has_morph_member : std::false_type { };

template<class T>
struct has_morph_member<T, std::void_t<decltype(T::morph)>> : std::true_type { };

/** \brief An index which is salted to provide a unique identifier for an element.
 *
 * \tparam DatumType The value type to which this identity refers.
 */
template<class DatumType = void>
struct identity;

template<class T>
struct is_identity_dereferenceable : std::false_type { };

template<class DatumType>
struct is_identity_dereferenceable<identity<DatumType>>
    : has_array_member<identity_traits<DatumType>> { };

template<class T>
inline constexpr
bool is_identity_dereferenceable_v = is_identity_dereferenceable<T>::value;

template<class DatumType>
DatumType& dereference_identity(const identity<DatumType>& id, std::false_type)
{
    using traits = identity_traits<DatumType>;
    return traits::array()[id.index()];
}

template<class DatumType>
DatumType& dereference_identity(const identity<DatumType>& id, std::true_type)
{
    using traits = identity_traits<DatumType>;
    return traits::morph(traits::array()[id.index()]);
}

template<class DatumType>
DatumType& dereference_identity(const identity<DatumType>& id)
{
    using traits = identity_traits<DatumType>;
    /*
    if constexpr (has_morph_member<traits>::value)
        return traits::morph(traits::array()[id.index()]);
    else
        return traits::array()[id.index()];
    */
    return dereference_identity(id, has_morph_member<traits>());
}

// --------------------------------
// Identity

template<class DatumType>
struct identity {
    using value_type = DatumType;
    using pointer    = typename std::add_pointer<DatumType>::type;
    using reference  = typename std::add_lvalue_reference<DatumType>::type;

    identity_raw raw; ///< The salt is stored in the high word, index in the low word.
                      ///< If \a raw is `-1`, then this identity refers to no element.

    /** \brief Retrieves the signed index of the identity.
     */
    constexpr identity_index index() const noexcept
    { return static_cast<identity_index>(static_cast<uint16>(raw)); } // C++20

    /** \brief Retrieves the salt of this identity. */
    constexpr identity_salt salt() const noexcept
    { return static_cast<identity_salt>(raw >> (CHAR_BIT * sizeof(identity_index))); }

    /** \brief Returns `true` if and only if this identity refers to an element. */
    constexpr bool valid() const noexcept { return raw != -1u; }

    /** \brief Returns `true` if and only if this identity refers to an element. */
    explicit constexpr operator bool() const noexcept { return valid(); }

    /** \brief Compares identities. */
    template<class S>
    constexpr bool
    operator==(identity<S> const& other) const noexcept
    { return raw == other.raw; }

    template<class S>
    constexpr bool
    operator!=(identity<S> const& other) const noexcept
    { return raw != other.raw; }

    /** \brief Conversion to an identity for the `base_type` specified in the identity
     *         traits, if supplied.
     */
    template<class U = std::remove_cv_t<DatumType>>
    constexpr operator identity<typename identity_traits<U>::base_type>() const noexcept
    { return {raw}; }

    /** \brief Conversion to an identity for the `base_type` specified in the identity
     *         traits, if supplied.
     */
    template<class U = std::remove_cv_t<DatumType>>
    constexpr identity<typename identity_traits<U>::base_type>
    to_base() const noexcept { return {raw}; }

    template<
        class Base,
        std::enable_if_t<std::is_base_of_v<Base, DatumType>, int> = 0
    > constexpr operator identity<Base>() const noexcept { return {raw}; }

    constexpr operator identity<void>() const noexcept { return {raw}; }

    /** \brief Constructs an identity from its index-salt pair. */
    static
    constexpr identity piecewise(identity_index index, identity_salt salt) noexcept
    { return {static_cast<uint16>(index) | (static_cast<uint32>(salt) << 16)}; }

    /** \brief Provides access to the element to which this identity refers.
     *
     * This identity is assumed to be valid, as by #valid.
     * If this identity is invalid, then this invokes undefined behavior.
     *
     * The program will fail to compile if \a DatumType, without cv-qualifiers, has no
     * compatible `dereference` traits method as stated in \ref identity_traits.
     *
     * \return A reference to the element to which this identity refers.
     */
    template<class U = std::remove_cv_t<DatumType>>
    std::enable_if_t<is_identity_dereferenceable_v<identity<U>>, reference>
    operator*() const { return dereference_identity(*this); }

    /** \brief Provides access to the element to which this identity refers.
     *
     * This identity is assumed to be valid, as by #valid.
     * If this identity is invalid, then this invokes undefined behavior.
     *
     * The program will fail to compile if \a DatumType, without cv-qualifiers, has no
     * compatible `dereference` traits method as stated in \ref identity_traits.
     *
     * \return A pointer to the element to which this identity refers.
     */
    template<class U = std::remove_cv_t<DatumType>>
    std::enable_if_t<is_identity_dereferenceable_v<identity<U>>, pointer>
    operator->() const { return &(this->operator*()); }
}; static_assert(sizeof(identity<void>)  == sizeof(identity_raw));
   static_assert(alignof(identity<void>) == alignof(identity_raw));

struct invalid_identity_type {
    template<class DatumType>
    constexpr operator identity<DatumType>() const noexcept { return {-1u}; }
};

inline constexpr invalid_identity_type invalid_identity = {};

static_assert(identity<void>{0xF0000BA7}.index() == 0x0BA7); // sanity check
static_assert(identity<void>{0xF0000BA7}.salt()  == 0xF000); // sanity check

/** @} */

}

namespace sentinel::literals {

constexpr signature operator "" _hsig(char const* str, std::size_t const len) noexcept {
    auto get = [&str, len] (std::size_t i) noexcept -> std::int32_t { return i < len ? str[i] : '\x20'; };
    return (signature)get(3u)
         | ((signature)get(2u) << 8)
         | ((signature)get(1u) << 16)
         | ((signature)get(0u) << 24);
}

}
