
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentutil/tag.hpp>

#include <algorithm>
#include <iterator>
#include <string_view>

#include <sentutil/globals.hpp>

namespace sentutil { namespace tag {

sentinel::tag_array_element* get_tag_meta_data(sentinel::signature type,
                                               const char* name)
{
    using sentinel::tag_array_element;
    if (name == nullptr)
        return nullptr;

    auto begin = std::begin(globals::tags_array);
    auto end   = std::end(globals::tags_array);
    auto it = std::find_if(begin, end,
                           [type, name = std::string_view(name)] (auto& tag)
                           { return tag.has_type(type) && tag.name == name; });

    return it != end ? &*it : nullptr;
}

} } // namespace sentutil::tag

