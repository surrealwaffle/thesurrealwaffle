#ifndef SENTINEL__STRUCTURES__BIPED_HPP
#define SENTINEL__STRUCTURES__BIPED_HPP

#include <sentinel/types.hpp>
#include <sentinel/structures/unit.hpp>

namespace sentinel {

struct biped_datum {
    int32 unknown00[3];

    index_long cbsp_surface_index; ///< The index of the surface in the map collision BSP that the unit has purchase.

    int32 unknown01[29];
}; static_assert(sizeof(biped_datum) == 0x84);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(biped_datum, cbsp_surface_index) == 0x0C);
#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

struct biped : unit {
    biped_datum biped;
}; static_assert(sizeof(biped) == 0x550);

}

#endif // SENTINEL__STRUCTURES__BIPED_HPP
