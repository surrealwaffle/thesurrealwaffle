#pragma once

#include "types.hpp"

namespace sentinel {

struct object_marker_result_type;
struct object_prototype_type;

} // namespace sentinel

namespace reve { namespace object {

using sentinel::object_marker_result_type;
using sentinel::object_prototype_type;

/** \brief Retrieves the markers of name \a marker_name for the object by \a identity.
 *
 * \sa proc_GetObjectMarkers
 *
 * \param[in] identity The object identifier.
 * \param[in] marker_name The name of the markers.
 * \param[out] out Stores the results.
 * \param[out] out_count The maximum number of elements to output through \a out.
 *
 * \return The number of markers stored through \a out.
 */
using get_object_markers_tproc __attribute__((cdecl))
    = h_short(*)(P_IN  identity_raw               identity    /*STACK*/,
                 P_IN  h_ccstr                    marker_name /*STACK*/,
                 P_OUT object_marker_result_type* out         /*STACK*/,
                 P_OUT h_short                    out_count   /*STACK*/);

/** \brief Retrieves the camera position for the object by \a id.
 *
 * \sa GetObjectCameraPosition
 *
 * \param[in] unit_id  The unit identity.
 * \param[in] position Stores the result.
 */
using get_unit_camera_position_tproc
    = exotic_function<void(P_IN  identity_raw unit_id  /*[ECX]*/,
                           P_OUT real*        position /*[EDI]*/)>;

extern get_object_markers_tproc       proc_GetObjectMarkers;
extern get_unit_camera_position_tproc proc_GetUnitCameraPosition; // exotic

extern object_prototype_type** ptr_pObjectPrototypes;

/** \brief Retrieves the camera position for the object by \a identity.
 *
 * \param[in] id The object identifier.
 * \param[in] position Stores the result.
 */
void GetUnitCameraPosition(identity_raw identity, real* out);

bool Init();

void Debug();

} } // namespace reve::object
