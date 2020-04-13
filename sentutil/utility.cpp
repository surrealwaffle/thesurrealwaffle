#include <sentutil/utility.hpp>
#include <sentutil/script.hpp>

#include <utility> // std::move
#include <vector>  // std::vector

namespace {

std::vector<sentutil::utility::handle> managed_handles;

} // namespace (anonymous)

namespace sentutil { namespace utility {

handle& handle::operator=(handle&& other) noexcept
{
    free_resource();
    res       = other.res;
    other.res = nullptr;
    return *this;
}

void handle::free_resource() noexcept
{
    sentinel_FreeHandle(res);
    res = nullptr;
}

bool handle::manage_handle(handle&& h)
{
    if (h) {
        managed_handles.push_back(std::move(h));
        return true;
    } else {
        return false;
    }
}

void clear_managed_handles()
{
    managed_handles.clear();
}

} } // namespace sentutil::utility
