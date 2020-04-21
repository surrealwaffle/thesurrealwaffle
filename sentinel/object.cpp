
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentinel/object.hpp>

#include "reve/object.hpp"
#include "reve/engine.hpp"

SENTINEL_API
int
sentinel_Object_GetMarkers(sentinel::identity<sentinel::object_table_datum> object,
                           sentinel::h_ccstr                    marker_name,
                           sentinel::object_marker_result_type* out,
                           sentinel::h_short                    out_count)
{
    return reve::object::proc_GetObjectMarkers(object.raw,
                                               marker_name,
                                               out, out_count);
}

SENTINEL_API
void
sentinel_Unit_GetCameraPosition(sentinel::identity<sentinel::unit> unit,
                                sentinel::position3d* position)
{
    reve::object::GetUnitCameraPosition(unit.raw, position->data());
}

SENTINEL_API
void
sentinel_Object_UpdateObjects(long ticks)
{
    for (; ticks > 0; --ticks) {
        reve::engine::proc_UpdateNetgameFlags();
        reve::engine::proc_UpdateObjects();
    }
}

SENTINEL_API
bool
sentinel_Engine_ExtrapolateLocalUnitDelta(sentinel::real3d* delta,
                                          sentinel::real3d* unknown0,
                                          sentinel::real3d* unknown1,
                                          sentinel::real    seconds)
{
    return reve::engine::proc_ExtrapolateLocalUnitDelta(delta->array,
                                                        unknown0->array,
                                                        unknown1->array,
                                                        seconds);
}

SENTINEL_API
void
sentinel_Biped_UpdateBipedPosition(sentinel::identity<sentinel::biped> biped_id)
{
    if (!biped_id)
        return;

    sentinel::flags_short control_flags = {/*ZERO INITIALIZED*/};
    control_flags |= (biped_id->unit.more_flags & 1) << CHAR_BIT;

    if (!(biped_id->object.damage_flags & 4)) // alive
        reve::engine::proc_GetBipedUpdatePositionFlags(biped_id.raw, &control_flags);

    reve::engine::proc_UpdateBipedPosition(biped_id.raw, &control_flags);
}
