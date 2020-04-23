
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentutil/simulation.hpp>

#include <cstring> // std::memcpy

#include <sentinel/object.hpp>
#include <sentutil/globals.hpp>

namespace sentutil {

simulation::simulation()
    : size_(static_cast<std::size_t>(globals::allocator_globals->allocated))
    , data_(new char[size_])
{
    std::memcpy(data_.get(), globals::allocator_globals->base, size_);
}

void simulation::advance(long ticks)
{
    using globals::runtime_sound_globals;
    if (ticks > 0) {
        // disable sound, store previous state
        bool is_sound_disabled = runtime_sound_globals->no_sound;
        runtime_sound_globals->no_sound = true;

        // advance objects by number of ticks
        sentinel_Object_UpdateObjects(ticks);

        // restore sound_enabled state to previous value
        runtime_sound_globals->no_sound = is_sound_disabled;
    }
}

void simulation::restore()
{
    if (data_)
        std::memcpy(globals::allocator_globals->base, data(), size());
}

} // namespace sentutil
