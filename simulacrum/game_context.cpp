#include "game_context.hpp"

#include <algorithm>

#include <sentutil/all.hpp>

namespace {

short ticks_between_fire = 2;

} // namespace (anonymous)

namespace simulacrum {

game_context_type game_context = {};

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
