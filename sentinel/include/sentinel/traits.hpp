
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits> // std::bool_constant, std::is_same, std::false_type,
                       // std::void_t

#include <sentinel/fundamental_types.hpp>

namespace sentinel::traits {

/** Defines static, constexpr `value` as `true` if \a T meets the requirements to be a table array element,
 *  otherwise defines `value` to `false`.
 */
template<class T, class = std::void_t<>>
struct is_table_element : std::false_type { };

template<class T>
struct is_table_element<
    T,
    std::void_t<decltype(&T::salt)>
> : std::bool_constant<std::is_same_v<decltype(&T::salt), identity_salt T::*>> { };

} // namespace sentinel::traits
