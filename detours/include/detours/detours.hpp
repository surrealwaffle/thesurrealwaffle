#pragma once

#include <functional>  // std::reference_wrapper
#include <iterator>    // std::back_inserter
#include <optional>    // std::optional
#include <string_view> // std::string_view
#include <utility>     // std::move
#include <vector>      // std::vector

#include <sigscan/sigscan.hpp>

#include "base.hpp"
#include "descriptors.hpp"
#include "patch.hpp"

namespace detours {

namespace management {

/** \brief Adds \a patches to a global collection of \ref patch objects.
 *
 * \return A \ref meta_patch used to control the managed patches.
 */
meta_patch manage_patches(std::vector<patch>&& patches);

/** \brief Destroys all managed patches passed through \ref manage_patches.
 *         The patches are destroyed in the reverse of the order they were supplied.
 */
void clear_managed_patches();

} // namespace detours::management

template<class Descriptor>
struct batch_descriptor {
    batch_descriptor() = delete;
    batch_descriptor(std::string_view  name,
                     const Descriptor& descriptor)
        : name(name)
        , descriptor(descriptor)
        , patch_writeback(std::nullopt){ }

    batch_descriptor(std::string_view  name,
                     const Descriptor& descriptor,
                     meta_patch&       patch_writeback)
        : name(name)
        , descriptor(descriptor)
        , patch_writeback(std::ref(patch_writeback)) { }

    batch_descriptor(batch_descriptor const&) = default;
    batch_descriptor(batch_descriptor&&)      = default;

    std::string_view name;
    Descriptor       descriptor;
    std::optional<std::reference_wrapper<meta_patch>> patch_writeback;
};

/** \brief Performs a scan for \a descriptor and, if the pattern is matched,
 *         performs the scan action, outputting any patches through \a patch_out.
 *
 * If \a descriptor is a \ref range_descriptor, then this function finds and patches
 * all occurrences of the descriptor's pattern.
 * Otherwise, this function finds and patches only the first occurrence.
 *
 * \return `true` if there was at least one match and all patches succeeded,
 *         otherwise `false`.
 */
template<class Descriptor, class OutputIt>
bool make_patch(Descriptor descriptor, OutputIt patch_out)
{
    static auto code_ranges = sigscan::get_text_segments();

    unsigned long number_matches = 0;
    unsigned long number_patches = 0;
    if (code_ranges) {
        for (auto range : *code_ranges) {
            while (auto pattern_instance = sigscan::scan_range(range, descriptor)) {
                ++number_matches;
                if (!perform_patch_action(descriptor, patch_out))
                    return false;
                ++number_patches;

                if constexpr (!is_range_descriptor<Descriptor>::value)
                    return true;
                else
                    range.first = pattern_instance->last;
            }
        }
    }

    return number_patches > 0 && number_matches == number_patches;
}

/** \brief Performs a scan for \a descriptor and, if the pattern is matched,
 *         performs the patch action, adding any patches to the patch manager.
 *
 * If this function returns `std::nullopt`, then any patches made during the call
 * are restored before returning to the caller.
 *
 * \return A \ref meta_patch of the patches performed, or
 *         `std::nullopt` if the scan or patch failed.
 */
template<class Descriptor>
std::optional<meta_patch> make_patch(const Descriptor& descriptor)
{
    std::vector<patch> patches;
    if (make_patch(descriptor, std::back_inserter(patches)))
        return management::manage_patches(std::move(patches));

    return std::nullopt;
}

template<class Descriptor>
std::optional<meta_patch> make_patch(std::reference_wrapper<Descriptor> descriptor)
{
    return make_patch(descriptor.get());
}

template<class Descriptor>
bool make_patch(const batch_descriptor<Descriptor>& d)
{
    if (d.patch_writeback) {
        if (auto p = make_patch(unwrap(d.descriptor))) {
            d.patch_writeback->get() = std::move(*p);
            return true;
        } else {
            return false;
        }
    } else {
        return static_cast<bool>(make_patch(d.descriptor));
    }
}

/** \brief Applies \ref make_patch to each descriptor supplied.
 *
 * All patches made are added to the manager.
 * If a patch or scan fails, then the remaining descriptors are left undone.
 *
 * \return The name of the first descriptor to fail \ref make_patch, otherwise
 *         `std::nullopt` if all scans and patches are successful
 */
template<class... Descriptors>
std::optional<std::string_view>
batch_patches(const batch_descriptor<Descriptors>&... descriptors)
{
    // patch_name is the name of the failed patch, or std::nullopt if no failure
    std::optional<std::string_view> patch_name = std::nullopt;

    auto try_patch = [&patch_name] (const auto& d) {
        if (make_patch(d)) return true;
        else               return (patch_name = d.name, false);
    };

    (void)(try_patch(descriptors) && ...);

    return patch_name;
}

} // namespace detours
