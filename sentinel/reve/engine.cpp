#include "engine.hpp"

#include <sentinel/config.hpp>

namespace reve { namespace engine {

update_netgame_flags_tproc proc_UpdateNetgameFlags = nullptr;
update_objects_tproc       proc_UpdateObjects      = nullptr;

extrapolate_local_unit_delta_tproc    proc_ExtrapolateLocalUnitDelta   = nullptr;
get_biped_update_position_flags_tproc proc_GetBipedUpdatePositionFlags = nullptr;
update_biped_position_tproc           proc_UpdateBipedPosition         = nullptr;

bool Init()
{
    return proc_UpdateNetgameFlags
        && proc_UpdateObjects
        && proc_ExtrapolateLocalUnitDelta
        && proc_GetBipedUpdatePositionFlags
        && proc_UpdateBipedPosition;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", proc_UpdateNetgameFlags);
    SENTINEL_DEBUG_VAR("%p", proc_UpdateObjects);
    SENTINEL_DEBUG_VAR("%p", proc_ExtrapolateLocalUnitDelta);
    SENTINEL_DEBUG_VAR("%p", proc_GetBipedUpdatePositionFlags);
    SENTINEL_DEBUG_VAR("%p", proc_UpdateBipedPosition);
}

} } // namespace reve::engine
