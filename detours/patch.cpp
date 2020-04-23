
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <detours/patch.hpp>

#include <algorithm> // std::all_of, std::copy, std::copy_n
#include <iterator>  // std::cbegin, std::cend
#include <utility>   // std::move

#include <sigscan/memory_range.hpp>

namespace detours {

patch::patch(patch&& other)
    : site(other.site)
    , restore_data(std::move(other.restore_data))
    , patch_data(std::move(other.patch_data))
    , success(other.success)
{
    other.site    = nullptr;
    other.success = false;
}

patch& patch::operator=(patch&& other)
{
    restore();
    site         = other.site;
    restore_data = std::move(other.restore_data);
    patch_data   = std::move(other.patch_data);
    success      = other.success;

    other.site    = nullptr;
    other.success = false;

    return *this;
}

patch::patch(byte* site, const byte* patch_data, std::ptrdiff_t size) noexcept
    : site(site)
    , restore_data()
    , patch_data()
    , success(false)
{
    if (patch_data)
        this->patch_data.insert(this->patch_data.cend(),
                                patch_data, patch_data + size);

    sigscan::memory_range site_range{this->site, this->site + size};
    if (site) {
        if (auto guard = sigscan::hold_range_rwx(site_range); !guard)
            return;
        else
            restore_data.insert(restore_data.cend(),
                                site_range.first, site_range.last);
    }

    repatch();
}

void patch::restore() noexcept
{
    if (!(*this))
        return;

    try {
        sigscan::memory_range site_range{site, site + restore_data.size()};

        if (auto guard = sigscan::hold_range_rwx(site_range)) {
            std::copy(std::cbegin(restore_data), std::cend(restore_data), site);
            sigscan::flush_range(site_range);
        }
    } catch (...) { /* DO NOTHING */ }

    restore_data.clear();
    success = false;
}

bool patch::repatch() noexcept
{
    if (site == nullptr || patch_data.empty())
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
    } catch (...) { /* DO NOTHING */ }

    return static_cast<bool>(*this);
}

bool meta_patch::is_patched() const noexcept
{
    return std::all_of(std::cbegin(patches), std::cend(patches),
                       [] (const patch& p) { return p.is_patched(); });
}

void meta_patch::restore() noexcept
{
    for (patch& p : patches)
        p.restore();
}

bool meta_patch::repatch() noexcept
{
    for (patch& p : patches) {
        if (!p.repatch())
            return false;
    }
    return true;
}

} // namespace detours
