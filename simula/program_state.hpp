#ifndef SIMULA__PROGRAM_STATE_HPP
#define SIMULA__PROGRAM_STATE_HPP

#include "nav.hpp"

#include <sentinel/types.hpp>
#include <sentinel/control.hpp>

namespace sentinel {
    struct raycast_result;
}

namespace simula {

struct program_state_t {
    nav::graph nav_graph;

    struct targeting_t {
        bool               has_target;
        sentinel::identity target;
        sentinel::real3    target_offset;
        char               target_marker[32];

        void clear() { *this = targeting_t{}; target.raw = -1; }
        void update(program_state_t& program_state, float const dt, long const ticks);

        bool set_target(sentinel::raycast_result const& raycast_result);
    } targeting;

    struct navigation_t {
        bool               has_target;
        sentinel::identity target;               // UNUSED
        sentinel::real3    target_offset;
        sentinel::int32    surface;              // UNUSED
        sentinel::int32    last_navable_surface; ///< The last navable surface that the player was on.

        void clear() { *this = navigation_t{}; target.raw = -1; surface = -1; last_navable_surface = -1; }
        void update(program_state_t& program_state, float const dt, long const ticks);

        bool set_target(program_state_t const& program_state, sentinel::raycast_result const& raycast_result);
    } navigation;

    struct controls_t {
        sentinel::control_state state;
        bool                    digital_held[20];
        bool                    is_moving_forward;
        bool                    is_moving_left;

        void clear() { *this = controls_t{}; }
    } controls;

    struct {
        bool enabled = true; ///< Set to `true` if and only if simula controls the player.
        bool noisy   = true; ///< Set to `true` if and only if simula may talk through chat.

        bool reset_targeting_on_death  = true;
        bool reset_navigation_on_death = false;
    } persistent;

    void stop();
    void reset();

    void update(float const dt, long const ticks);
} extern program_state;

inline void program_state_t::stop() {
    targeting.clear();
    navigation.clear();
    controls.clear();
}

inline void program_state_t::reset() {
    nav_graph.clear();
    stop();
    nav_graph.build();
}

inline void program_state_t::update(float const dt, long const ticks) {
    if (!persistent.enabled)
        return;

    targeting.update(*this, dt, ticks);
    navigation.update(*this, dt, ticks);
}

}


#endif // SIMULA__PROGRAM_STATE_HPP
