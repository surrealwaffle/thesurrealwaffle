#pragma once

#include <sentinel/types.hpp>
#include <sentinel/globals.hpp>
#include <sentinel/structures/tag.hpp>

namespace sentutil { namespace tag {

/** \brief Finds a tag and retrieves its meta data.
 *
 * The following code retrieves the weapon tag (`.weapon`) for the default pistol.
 * \code{.cpp}
   // if the sentinel::literals namespace has been imported, one can use
   // "weapon"_hsig in place of sentinel::make_signature
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
sentinel::tag_array_element* get_tag_meta_data(sentinel::signature type,
                                               const char* name);

} } // namespace sentutil::tag

