
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "sound.hpp"

#include <cstdio>

#include <sentinel/config.hpp>

namespace reve { namespace sound {

direct_sound_interfaces* ptr_DirectSoundInterfaces = nullptr;
secondary_sound_buffers* ptr_SecondarySoundBuffers = nullptr;

bool Init()
{
    return ptr_DirectSoundInterfaces
        && ptr_SecondarySoundBuffers;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", ptr_DirectSoundInterfaces);
    SENTINEL_DEBUG_VAR("%p", ptr_SecondarySoundBuffers);
}

} } // namespace reve::sound
