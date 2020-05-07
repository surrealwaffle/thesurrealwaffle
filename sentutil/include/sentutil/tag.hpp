
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

#include <sentinel/types.hpp>
#include <sentinel/globals.hpp>
#include <sentinel/structures/tag.hpp>

namespace sentutil { namespace tag {

/** \brief Finds a tag and retrieves its meta data.
 *
 * The following code retrieves the weapon tag (`.weapon`) for the default pistol.
 * \code{.cpp}
   // if the sentinel::literals namespace has been imported, one can use
   // "weapon"_hsig in place of `sentinel::make_signature("weapon")`
   auto pistol_meta_data = get_tag_meta_data(sentinel::make_signature("weapon"),
                                             "weapons/pistol/pistol");
 * \endcode
 *
 * \param[in] type The signature of the tag type.
 * \param[in] name The name of the tag.
 *
 * \return A pointer to the tag data of specified type and name, if it exists, or
 *         `nullptr` if the tag could not be found or \a name was `nullptr`.
 */
sentinel::tag_array_element*
get_tag_meta_data(sentinel::signature type, const char* name);

sentinel::tag_array_element*
get_tag_meta_data(const sentinel::identity<sentinel::tag_array_element>& id);

template<class Tag>
typename std::enable_if<
    sentinel::is_tag_v<Tag>,
    sentinel::tag_array_element*
>::type
get_tag_meta_data(const sentinel::identity<Tag>& id)
{
    sentinel::tag_array_element* tag = get_tag_meta_data(id.to_base());
    if (tag && !tag->has_type(sentinel::identity_traits<Tag>::type_signature))
        tag = nullptr; // type does not match
    return tag;
}

} } // namespace sentutil::tag

