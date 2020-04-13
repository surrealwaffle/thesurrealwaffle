#pragma once

#include <cstddef> // std::size_t

#include <array>      // std::array
#include <functional> // std::reference_wrapper
#include <utility>    // std::forward
#include <vector>     // std::vector

#include <sigscan/memory_range.hpp>

#include "base.hpp"

namespace detours {

struct assign_value_type { };

inline constexpr assign_value_type assign_value = {};

/** \brief An RAII wrapper that maintains edits on a region of memory.
 */
class patch {
public:
    patch(const patch&)            = delete; ///< DELETED
    patch& operator=(const patch&) = delete; ///< DELETED

    /** \brief Initializes an empty patch.
     */
    patch() : site(nullptr), restore_data(), success(false) { }

    /** \brief Moves ownership of the patch from \a other.
     */
    patch(patch&& other);

    /** \brief Restores this patch and moves ownership of the patch from \a other.
     */
    patch& operator=(patch&& other);

    /** \brief Creates a patch of \a size bytes at \a site with \a patch_data.
     *
     * If \a patch_data is `nullptr`, no patch is made but \a size bytes are stored
     * in the restore data buffer.
     *
     * This constructor temporarily makes the patch site writable.
     */
    patch(byte* site, const byte* patch_data, std::ptrdiff_t size) noexcept;

    /** \brief Commits the patch from a `std::array` of bytes.
     */
    template<std::size_t N>
    patch(byte* site, const std::array<byte, N>& patch_data)
        : patch(site, patch_data.cbegin(), N) { }

    template<class T, class Value>
    patch(assign_value_type, T* pointer, Value&& value);

    /** \brief If this object owns a patch, restores the original data.
     */
    ~patch() noexcept { restore(); }

    /** \brief Returns `true` if the patch is successful, otherwise `false`.
     */
    bool is_patched() const noexcept { return success && site != nullptr; }

    /** \brief Returns `true` if the patch is successful, otherwise `false`.
     */
    explicit operator bool() const noexcept { return is_patched(); }

    /** \brief Restores the original data at the patched site if this object is
     *         holding modifications on a site, otherwise does nothing.
     */
    void restore() noexcept;

    /** \brief Attempts to repatch the site supplied during construction.
     *
     * \return `true` if the patch is successful, otherwise `false`.
     */
    bool repatch() noexcept;

private:
    byte*             site;
    std::vector<byte> restore_data;
    std::vector<byte> patch_data;
    bool              success;
};

/** \brief A non-owning collection of \ref patch objects that mocks the interface
 *         of a \ref patch.
 */
class meta_patch {
public:
    meta_patch() = default;

    template<class ForwardIt>
    meta_patch(ForwardIt first, ForwardIt last) : patches(first, last) { }

    /** \brief Returns `true` if all patches are successful, otherwise `false`.
     */
    bool is_patched() const noexcept;

    /** \brief Returns `true` if all patches are successful, otherwise `false`.
     */
    explicit operator bool() const noexcept { return is_patched(); }

    /** \brief Restores the original data of the patch sites.
     */
    void restore() noexcept;

    /** \brief Attempts to reapply the patches.
     *
     * \return `true` if the patches are successful, otherwise `false`.
     */
    bool repatch() noexcept;

private:
    std::vector<std::reference_wrapper<patch>> patches;
};

// -----------------------------------------------------------------------------------

/*
bool patch::repatch() noexcept
{
    if (site == nullptr)
        return false;

    const bool set_restore_point = !is_patched();
    success = false;
    try {
        const auto size = patch_data.size();
        sigscan::memory_range site_range{site, site + size};

        if (auto guard = sigscan::hold_range_rwx(site_range)) {
            // set restore_data
            if (set_restore_point) {
                restore_data.clear();
                restore_data.reserve(size);
                std::copy_n(site, size, std::back_inserter(restore_data));
            }

            // commit the patch
            std::copy_n(std::cbegin(patch_data), size, site);
            sigscan::flush_range(site_range);

            success = true;
        }
    } catch (...) { }

    return static_cast<bool>(*this);
}
*/

template<class T, class Value>
patch::patch(assign_value_type, T* pointer, Value&& value)
    : patch(reinterpret_cast<byte*>(pointer), nullptr, sizeof(T))
{
    if (site == nullptr)
        return;

    try {
        constexpr auto size = sizeof(T);
        sigscan::memory_range site_range{site, site + size};

        if (auto guard = sigscan::hold_range_rwx(site_range)) {
            *pointer = std::forward<Value>(value);
            patch_data.insert(patch_data.cend(), site, site + size);
            success = true;
        }
    } catch (...) { /* DO NOTHING */}
}

} // namespace detours

