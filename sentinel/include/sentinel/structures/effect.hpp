#ifndef SENTINEL__STRUCTURES__EFFECT_HPP
#define SENTINEL__STRUCTURES__EFFECT_HPP

#include <sentinel/types.hpp>
#include <sentinel/tags/effect.hpp>

namespace sentinel {

struct effect {
    identity_salt salt;
    int16         unknown00;
    identity<tags::effect>      tag;
    int32         unknown01[61];
}; static_assert(sizeof(effect) == 0xFC);

}

#endif
