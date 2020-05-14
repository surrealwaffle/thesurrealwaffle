
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "game_context.hpp"

#include <cmath>

#include <algorithm>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

#include <sentutil/all.hpp>

namespace {

std::optional<std::string> ignore_name_prefix;

/** \brief Returns `true` if \a name matches \a string, with non-representable
 *         characters ignored from \a name.
 */
bool name_matches(const std::wstring_view name, const std::string_view string);

} // namespace (anonymous)

#define SIMULACRUM_USE_CLOSED_FORM_TRAVEL_COMPUTATION

namespace simulacrum {

OrientationContext::OrientationContext(float yaw, float pitch)
    : yaw(yaw)
    , pitch(pitch)
    , cos_yaw(std::cos(yaw))
    , sin_yaw(std::sin(yaw))
    , cos_pitch(std::cos(pitch))
    , sin_pitch(std::sin(pitch)) { }

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

If we wish to do away with the std::sqrt entirely, we can massage it and perform
a Taylor expansion of v_i * sqrt(1 + u), where u = (2 * K)/(v_i)^2 * x.
    We can guarantee |u| < 1 for values of x in [0, (r_f - r_i)).
    This may be entirely unnecessary, so only implement it as a last resort.
*/

ProjectileContext::ProjectileContext(const sentinel::tags::projectile& projectile) noexcept
    : definition(projectile)
    , destroyed_at_final_speed(projectile.projectile.detonation.maximum_range <= 0.0f)
    , does_lerp(projectile.does_lerp())
    , speed_muzzle(projectile.projectile.physics.initial_velocity)
    , speed_final(projectile.projectile.physics.final_velocity)
    , damage_range(projectile.projectile.physics.air_damage_range)
    , detonation_range(projectile.projectile.detonation.maximum_range)
    , max_range(destroyed_at_final_speed && does_lerp ? lerp_distance : detonation_range)
    , lerp_distance(damage_range.length())
    , lerp_time(does_lerp ? 2 * lerp_distance / (speed_final + speed_muzzle) : 0.0f)
    , lerp_constant(does_lerp ? (speed_final - speed_muzzle) / lerp_time : 0.0f)
    , reciprocal_lerp_constant(does_lerp ? 1 / lerp_constant : 0.0f)
    , half_lerp_constant(lerp_constant / 2)
{ }

bool GameContext::load()
{
    using sentutil::script::install_script_function;
    return
        install_script_function<"simulacrum_ignore_name">(
            +[] (std::optional<std::string_view> name) {
                if (!name) {
                    ignore_name_prefix = std::nullopt;
                    return;
                }

                ignore_name_prefix = std::make_optional(std::string(name.value()));
            }) &&
        install_script_function<"simulacrum_held_weapon_tag">(
            +[] {
                if (!game_context.weapon_id) {
                    sentutil::console::cprint("no active weapon");
                    return;
                }

                const sentinel::tag_array_element& element = *game_context.weapon_id->object.tag;
                sentutil::console::cprintf("id: %08X", element.identity.raw);
                sentutil::console::cprintf("path: \"%s\"", element.name);
            });
}

void GameContext::preupdate(long ticks)
{
    // avoid overflow
    if ((ticks_since_fired += ticks) > 1000L)
        ticks_since_fired = 1000L;

    can_fire_primary_trigger = can_fire_secondary_trigger = can_throw_grenade = false;

    weapon_id          = sentinel::invalid_identity;
    weapon             = std::nullopt;
    weapon_definition  = std::nullopt;
    projectile_context = std::nullopt;
    weapon_config      = std::nullopt;

    local_player = std::nullopt;
    local_unit   = std::nullopt;
    orientation_context = {};
    players.clear();
    allies = {};
    enemies = {};
    live_allies = {};
    live_enemies = {};

    auto player_filter = [] (sentinel::player& player) {
        return (!ignore_name_prefix || !name_matches(player.name, ignore_name_prefix.value()));
    };

    for (sentinel::player& player : sentutil::globals::players) {
        if (player.is_local())
            local_player = player;
        else if (player_filter(player))
            players.push_back(player);
    }

    if (!local_player)
        return;

    if (local_player.value().get().unit) {
        local_unit = std::ref(*local_player.value().get().unit);

        weapon_id = local_player.value().get().unit->get_weapon();
        if (weapon_id) {
            sentinel::weapon&       weapon_ref = *weapon_id;
            sentinel::tags::weapon& weapon_def = get_definition(weapon_ref);

            weapon = std::ref(weapon_ref);
            weapon_definition = std::ref(weapon_def);

            if (weapon_def.weapon.triggers.count) {
                sentinel::tags::weapon_definition::trigger& trigger = weapon_def.weapon.triggers[0]; // works for most cases
                if (trigger.projectile.projectile)
                    projectile_context = ProjectileContext(*trigger.projectile.projectile);
            }
        }
    }
    weapon_config = std::ref(config::get_config_state().get_weapon_config(weapon_id));

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

    if (local_unit) {
        {   // sort live allies/enemies based on (square) distance
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
    } else { // player is dead
        ticks_since_fired = 1000L;
    }

    can_fire_primary_trigger = weapon && ticks_since_fired >= weapon_config.value().get().firing_interval;

    orientation_context = OrientationContext(sentutil::globals::local_player_globals->players[0].yaw,
                                             sentutil::globals::local_player_globals->players[0].pitch);
}

void GameContext::postupdate(const sentinel::digital_controls_state& digital)
{
    if (digital.primary_trigger && can_fire_primary_trigger) {
        ticks_since_fired = 0L;
    }
}

long
GameContext::get_ticks_until_fire() const
{
    return std::max(get_current_weapon_config().firing_interval - ticks_since_fired, 0L);
}

const config::WeaponConfig&
GameContext::get_current_weapon_config() const
{
    return weapon_config.value_or(config::get_config_state().default_weapon_config);
}

GameContext game_context = {};

} // namespace simulacrum

#undef SIMULACRUM_USE_CLOSED_FORM_TRAVEL_COMPUTATION

namespace {

bool name_matches(const std::wstring_view name, const std::string_view str)
{
    auto name_first = name.begin();
    auto name_last  = name.end();

    auto str_first = str.begin();
    auto str_last  = str.end();

    auto is_representable = [] (const wchar_t wc) { return wc <= std::numeric_limits<char>::max(); };

    for (name_first = std::find_if(name_first, name_last, is_representable);
         name_first != name_last && str_first != str_last && *name_first == *str_first;
         name_first = std::find_if(name_first + 1, name_last, is_representable), ++str_first)
    { /* DO NOTHING */ }

    return str_first == str_last;
}

} // namespace (anonymous)
