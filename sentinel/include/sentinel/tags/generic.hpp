#pragma once

#include <cstddef>

#include <type_traits>

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>

namespace sentinel { namespace tags {

/** \brief Reference to another tag found internally in tag metadata. */
template<class Tag>
struct tag_reference {
    using pointer   = typename std::add_pointer<Tag>::type;
    using reference = typename std::add_lvalue_reference<Tag>::type;

    signature     type;
    char const*   name; ///< The tag's path.
    int32         unknown; // x08
    identity<Tag> tag;  ///< The identity of the tag referred to by this reference.

    bool valid() const noexcept { return tag.valid(); }
    explicit operator bool() const noexcept { return valid(); }

    template<class U = std::remove_cv_t<Tag>>
    std::enable_if_t<is_identity_dereferenceable_v<identity<U>>, reference>
    operator*() const { return *tag; }

    template<class U = std::remove_cv_t<Tag>>
    std::enable_if_t<is_identity_dereferenceable_v<identity<U>>, pointer>
    operator->() const { return tag->operator->(); }
}; static_assert(sizeof(tag_reference<void>) == 0x10);

/** \brief Reference to an array of data found in tag metadata.
 *
 * Note that \a T is not necessarily a tag nor a tag reference.
 * For instance, netgame flags describe item, player, and objective spawns
 * and are more than just a tag reference.
 */
template<class T>
struct tag_block {
    using value_type = T;
    using pointer = T*;
    using const_pointer = T const*;

    using iterator = pointer;
    using const_iterator = const_pointer;

    int32   count;  ///< The number of elements in the array pointed to by #data.
    pointer data;   ///< The data array.
    void*   unknown;// x08 - not sure type or meaning

    iterator begin() noexcept { return data; }
    iterator end() noexcept { return data + count; }

    const_iterator begin() const noexcept { return data; }
    const_iterator end() const noexcept { return data + count; }

    const_iterator cbegin() const noexcept { return data; }
    const_iterator cend() const noexcept { return data + count; }

    template<bool B = std::is_same<value_type, void>()>
    auto operator[](std::size_t const index) noexcept
        -> std::enable_if_t<!B, T>& {
        return data[index];
    }

    template<bool B = std::is_same<value_type, void>()>
    auto operator[](std::size_t const index) const noexcept
        -> std::enable_if_t<!B, T> const& {
        return data[index];
    }
}; static_assert(sizeof(tag_block<void>) == 0x0C);

} } // namespace sentinel::tags
