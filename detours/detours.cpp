
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <detours/detours.hpp>

#include <algorithm> // std::for_each
#include <deque>     // std::deque
#include <iterator>  // std::back_inserter, std::make_move_iterator
#include <mutex>     // std::lock_guard, std::mutex

namespace {

std::mutex                 managed_patches_mtx;
std::deque<detours::patch> managed_patches;

} // namespace (anonymous)

namespace detours {

namespace management {

meta_patch manage_patches(std::vector<patch>&& patches)
{
    std::lock_guard guard(managed_patches_mtx);
    auto it = managed_patches.insert(std::end(managed_patches),
                                     std::make_move_iterator(std::begin(patches)),
                                     std::make_move_iterator(std::end(patches)));
    return meta_patch(it, std::end(managed_patches));
}

void clear_managed_patches()
{
    std::lock_guard guard(managed_patches_mtx);
    std::for_each(std::rbegin(managed_patches), std::rend(managed_patches),
                  [] (patch& p) { p.restore(); });
    managed_patches.clear();
}

} // namespace detours::management

} // namespace detours

#ifdef REFERENCE

#include <vector>
#include <mutex>
#include <utility>

#include <detours.hpp>
#include <detours/patch.hpp>

namespace {

    using detours::patch;

    std::vector<patch> patches;
    std::mutex         patches_mtx;

}

namespace detours::management {
    void AddToManagedPatches(std::vector<patch>&& patches_to_add) {
        std::lock_guard guard{patches_mtx};

        patches.reserve(patches.size() + patches_to_add.size());
        for (auto& patch : patches_to_add)
            patches.push_back(std::move(patch));

        patches_to_add.clear();
    }

    void ClearManagedPatches() {
        std::lock_guard guard{patches_mtx};

        // although we could use patches.clear(), we need to guarantee reverse-order destruction
        // patches.clear();

        while (!patches.empty())
            patches.pop_back();

    }
}

#endif
