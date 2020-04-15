#pragma once

#include <type_traits> // std::invoke_result
#include <utility>     // std::move
#include <string_view> // std::string_view

#include <sentinel/base.hpp>
#include <sentutil/impl/impl_utility.hpp>

namespace sentutil { namespace utility {

template<class ToPointer>
class heavy_pointer_wrapper
    : public impl::dereference_container_interface<heavy_pointer_wrapper<ToPointer>>
    , public impl::dereference_function_interface<heavy_pointer_wrapper<ToPointer>>
    , public impl::dereference_subscript_interface<heavy_pointer_wrapper<ToPointer>>
{
public:
    using pointer   = std::invoke_result_t<ToPointer&>;
    using reference = std::remove_pointer_t<pointer>;

    heavy_pointer_wrapper() = default;
    heavy_pointer_wrapper(ToPointer to_pointer) : to_pointer(to_pointer) { }

    pointer  operator->()   const { return to_pointer(); }
    reference dereference() const { return *(to_pointer()); }
    reference operator*()   const { return dereference(); }

    explicit operator bool() const { return this->operator->(); }

    using impl::dereference_container_interface<heavy_pointer_wrapper>::begin;
    using impl::dereference_container_interface<heavy_pointer_wrapper>::end;
    using impl::dereference_container_interface<heavy_pointer_wrapper>::cbegin;
    using impl::dereference_container_interface<heavy_pointer_wrapper>::cend;

    using impl::dereference_function_interface<heavy_pointer_wrapper>::operator();

    using impl::dereference_subscript_interface<heavy_pointer_wrapper>::operator[];

private:
    ToPointer to_pointer;
};

/** \brief Provide string literals as non-type template parameters.
 */
template<std::size_t N>
struct static_string {
    static_assert(sizeof(N) > 0);

    char string[N]; ///< The string itself.

    constexpr static_string() : string{} { }

    /** \brief Constructs the fixed string from the supplied string \a buf.
     */
    constexpr static_string(const char (&buf)[N]) : string{}
    {
        for (std::size_t i = 0; i < N; ++i)
            string[i] = buf[i];
    }

    /** \brief Conversion to pointer.
     */
    constexpr operator const char*() const { return string; }

    /** \brief Conversion to string view.
     */
    constexpr  operator std::string_view() const { return string; }

    constexpr char* begin() noexcept { return +string; }
    constexpr char* end() noexcept { return begin() + N; }

    constexpr const char* begin() const noexcept { return cbegin(); }
    constexpr const char* end() const noexcept { return cend(); }

    constexpr const char* cbegin() const noexcept { return +string; }
    constexpr const char* cend() const noexcept { return begin() + N; }

    template<std::size_t M>
    constexpr static_string<N + M>
    operator+(const static_string<M>& other) const
    {
        static_string<N + M> result;
        auto dst = result.begin();
        for (auto it = begin(); it != end() && *it != '\0'; ++it)
            *dst++ = *it;
        for (auto it = other.begin(); it != other.end(); ++it)
            *dst++ = *it;
        return result;
    }
};

/** \brief RAII wrapper for `sentinel_handle` resources.
 */
class handle {
public:
    /** \brief Constructs the wrapper to own no resource.
     */
    handle() noexcept : res{nullptr} { }

    /** \brief Wraps a `sentinel` resource, effectively taking ownership.
     */
    handle(sentinel_handle resource) noexcept : res(resource) { }

    /** \brief Transfers resource ownership.
     */
    handle(handle&& other) noexcept : res(other.res) { other.res = nullptr; }

    /** \brief Transfers resource ownership and releases the current resource, if any.
     */
    handle& operator=(handle&&) noexcept;

    handle(const handle&)            = delete; ///< DELETED
    handle& operator=(const handle&) = delete; ///< DELETED

    /** \brief Releases the owned resource (if any).
     */
    void free_resource() noexcept;

    /** \brief Releases the owned resource (if any).
     */
    ~handle() noexcept { free_resource(); }

    /** \brief Returns `true` if and only if this wrapper owns a resource.
     */
    explicit operator bool() const noexcept { return res; }

    /** \brief Adds a handle to be managed and freed when the program exits.
     *         The user may clear all managed handles via #clear_managed_handles.
     *
     * \return `true` if \a h manages a resource, otherwise `false`.
     */
    static bool manage_handle(handle&& h);

    /** \brief Frees all handles managed by #manage_handle.
     */
    static void clear_managed_handles();

    /** \brief Effectively calls `manage_handle(std::move(*this))`.
     */
    bool make_managed() { return manage_handle(std::move(*this)); }

private:
    ::sentinel_handle res; ///< The resource handle owned by the wrapper.
};

inline bool manage_handle(handle&& h) { return handle::manage_handle(std::move(h)); }

/** \brief Returns `true` if the `bool`s in \a ilist are already partitioned,
 *         with the `false` values occurring before the `true` values.
 */
inline constexpr bool are_bools_sorted(std::initializer_list<bool> ilist)
{
    bool prev = false;
    for (bool b : ilist) {
        if (prev && !b)
            return false;
        prev = b;
    }

    return true;
}

/** \brief Indicates that a formatting overload should be used instead.
 */
struct format_marker_type { };

/** \brief A value that can be passed when relevant to select a formatting overload.
 */
inline constexpr format_marker_type format_marker = {};

} } // namespace sentutil::utility

// ----------------------------------
// Exports to sentutil main namespace

namespace sentutil {

using utility::format_marker;

} // namespace sentutil
