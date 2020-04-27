
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "game_context.hpp"

#include <cmath>

#include <algorithm>

#include <sentutil/all.hpp>

namespace {

short ticks_between_fire = 2;

} // namespace (anonymous)

namespace simulacrum {

game_context_type game_context = {};

/*
We wish to determine the amount of time it will take for the projectile to reach a target distance, assuming the target is within distance.
    If max range is positive and non-negative, then the target must be within max range.
    If max range is 0, then the projectile terminates when it reaches final velocity at x = r_f - r_i, and the target must be within that distance.

We can opt to model the rate of change in velocity in the lerping interval over *time* (rather than space) as a constant K.
    K = (v_f - v_i)/(lerping time)
      = (v_f - v_i)/T,
    where T is chosen so that x(T)=r_f - r_i.

Then v(t) = {v_i + K * t, if t in [0, T];
            {v_f, if t >= T.

Then x(t) = {v_i * t + 1/2 * K * t^2, if t in [0, T];
            {(r_f - r_i) + v_f * (t - T), if t >= T.

Note: v_i * T + 1/2 * K * T^2 = v_i * T + 1/2 * (v_f - v_i) * T = (v_f + v_i)/2 * T = r_f - r_i, from the constraint on x(T).
    Then T = 2 * (r_f - r_i)/(v_f + v_i).
    Example: plasma rifle
        T = 2 * (50 - 20)/(25 + 50) = 4/5
        K = (25 - 50)/(4/5) = -31.25
        (remember that Halo time is in ticks and the above are taken in seconds, so divide T by 30 and K by 900)

This is ideal, as both T and K can be precomputed trivially without any expensive operations.
Actually, given numerical data, this is more accurate to how Halo operates.

1.7416666746139526370 to 1.7069445848464965820: -0.034722089767456055
1.6000001430511474610 to 1.5652778148651123050: -0.034722328186035156
    K (in proper units): -(31/25)/900 = -0.03472...

Suppose we have a target is a distance X away and is within range. For what value of t is the target reachable?
    If X < (r_f - r_i), then 1/2 * K * t^2 + v_i * t = X, so t = (-v_i + sqrt((v_i)^2 + 2 * K * X))/K;
    otherwise, if X >= (r_f - r_i), then (r_f - r_i) + v_f * (t - T) = X, or t = T + (X - (r_f - r_i))/v_f.
*/

projectile_context::projectile_context(const sentinel::tags::projectile& projectile) noexcept
    : projectile_(projectile)
    , detonates_at_final_velocity_(projectile.projectile.detonation.maximum_range <= 0.0f)
    , velocity_initial_(projectile.projectile.physics.initial_velocity)
    , velocity_final_(projectile.projectile.physics.final_velocity)
    , square_velocity_initial_(velocity_initial_ * velocity_initial_)
    , reciprocal_velocity_final_(1 / velocity_final_)
    , lerp_distance_(projectile.projectile.physics.air_damage_range.length())
    , lerp_time_(2 * lerp_distance_ / (velocity_initial_ + velocity_final_))
    , lerp_constant_((velocity_final_ - velocity_initial_) / lerp_time_)
    , reciprocal_lerp_constant_(1 / lerp_constant_)
{

}

sentinel::real projectile_context::max_range() const
{
    // TODO: account for inherit_velocity if detonates_at_final_velocity_ is true
    return detonates_at_final_velocity_ ? lerp_distance_ : projectile().projectile.detonation.maximum_range;
}

std::optional<sentinel::ticks_long> projectile_context::travel_ticks(const sentinel::real distance) const
{
    if (distance < max_range())
        return std::nullopt;
    else if (distance < lerp_distance_)
        return std::ceil(reciprocal_lerp_constant_ * (-velocity_initial_ + std::sqrt(square_velocity_initial_ + 2 * lerp_constant_ * distance)));
    else
        return std::ceil(lerp_time_ + reciprocal_velocity_final_ * (distance - lerp_distance_));
}

bool game_context_type::load()
{
    return sentutil::script::install_script_function<"simulacrum_ticks_between_fire">(
        +[] (std::optional<short> v) {
            if (v) ticks_between_fire = v.value();
            return ticks_between_fire;
        });
}

void game_context_type::preupdate(long ticks)
{
    ticks_since_fired    += ticks;
    ticks_until_can_fire = std::max(ticks_until_can_fire - ticks, 0L);

    can_fire_primary_trigger = can_fire_secondary_trigger = can_throw_grenade = false;

    local_player = std::nullopt;
    local_unit   = std::nullopt;
    players.clear();
    allies = {};
    enemies = {};
    live_allies = {};
    live_enemies = {};

    for (sentinel::player& player : sentutil::globals::players) {
        if (player.is_local())
            local_player = player;
        else
            players.push_back(player);
    }

    if (!local_player)
        return;

    if (local_player.value().get().unit)
        local_unit = std::ref(*local_player.value().get().unit);

    {   // sort players into ally/enemy partitions
        auto is_ally = [team = local_player.value().get().team]
                       (const sentinel::player& player)
                       { return team == player.team; };
        auto it = std::partition(players.begin(), players.end(), is_ally);
        allies = {players.begin(), it};
        enemies = {it, players.end()};
    }

    {   // sort ally/enemy partitions into alive/dead partitions
        auto is_alive = [] (sentinel::player& player) { return player.is_alive(); };
        live_allies = {allies.begin(),
                       std::partition(allies.begin(), allies.end(), is_alive)};
        live_enemies = {enemies.begin(),
                        std::partition(enemies.begin(), enemies.end(), is_alive)};
    }

    if (local_unit) { // sort live allies/enemies based on (square) distance
        auto get_position = [] (const sentinel::player& p) {
            const sentinel::unit& unit = *p.unit;
            return unit.object.parent ? unit.object.parent->object.position
                                      : unit.object.position;
        };
        auto cmp = [get_position, pos = get_position(local_player.value())]
                   (const sentinel::player& a, const sentinel::player& b)
                   { return norm2(get_position(a) - pos) < norm2(get_position(b) - pos); };
        std::sort(live_allies.begin(), live_allies.end(), cmp);
        std::sort(live_enemies.begin(), live_enemies.end(), cmp);
    }

    can_fire_primary_trigger = static_cast<bool>(local_unit)
                            && ticks_until_can_fire == 0;
}

void game_context_type::postupdate(const sentinel::digital_controls_state& digital)
{
    if (digital.primary_trigger && can_fire_primary_trigger) {
        ticks_until_can_fire = std::max(2, (int)ticks_between_fire); ///< \todo Perform a more convoluted calculation
        ticks_since_fired = 0;
    }
}

} // namespace simulacrum
