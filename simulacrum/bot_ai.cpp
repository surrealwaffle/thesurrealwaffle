
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "bot_ai.hpp"
#include "bot_control.hpp"
#include "game_context.hpp"
#include "graph.hpp"

#include <algorithm>
#include <functional>
#include <iterator>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/index/rtree.hpp>

#include <sentutil/all.hpp>

namespace {

simulacrum::navigation_graph nav_graph;

} // namespace (anonymous)

namespace simulacrum { namespace ai {

void reset()
{
    recalculate_navigation(std::nullopt);
}

bool load()
{
    return true;
}

void recalculate_navigation(std::optional<std::string_view> cache_name)
{
    nav_graph = simulacrum::navigation_graph(simulacrum::this_collision_bsp);
}

void update(float seconds, long ticks)
{
    if (!game_context.local_unit || game_context.live_enemies.empty()) {
        control::immediate_goals.clear();
        return;
    }

    [[maybe_unused]]
    sentinel::player& local_player = game_context.local_player.value();
    sentinel::unit&   local_unit   = game_context.local_unit.value();
    sentinel::biped&  local_biped  = reinterpret_cast<sentinel::biped&>(local_unit);

    sentinel::player& nearest_enemy_player = *game_context.live_enemies.begin();
    sentinel::unit&   nearest_enemy_unit   = *nearest_enemy_player.unit;

    auto get_position = [] (const sentinel::unit& unit) -> const sentinel::real3d& {
        return unit.object.parent ? unit.object.parent->object.position
                                  : unit.object.position;
    };

    control::immediate_goals.target_player = std::ref(nearest_enemy_player);

    auto start_vertex = [get_position, local_biped] {
        const navigation_graph_node node{{},
                                         navigation_graph_node::type_surface,
                                         local_biped.biped.cbsp_surface_index};
        auto opt = nav_graph.get_node(node);
        return opt ? opt.value() : nav_graph.nearest_node(get_position(local_biped));
    }();
    auto goal_vertex = nav_graph.nearest_node(get_position(nearest_enemy_unit));

    if (!start_vertex || !goal_vertex)
        return; // OK to use previous pathing goals

    auto heuristic = [] (const navigation_graph_node& node,
                         const navigation_graph_node& goal)
                         { return norm(goal.point - node.point); };

    auto visitor = [count = 0] (auto&&... ) mutable { return ++count < 30000; };

    auto test_edge = [] (const navigation_graph_node& start,
                         const navigation_graph_node& end,
                         const auto& edge) {
        return std::true_type();
    };

    auto search_result = astar_search(nav_graph.get_graph(),
                                      start_vertex.value(),
                                      goal_vertex.value(),
                                      heuristic,
                                      visitor,
                                      test_edge);
    auto path_opt = get_path(start_vertex.value(),
                             goal_vertex.value(),
                             search_result);

    if (!path_opt)
        return;

    auto& path = path_opt.value();
    auto& dest = control::immediate_goals.target_position;
    std::fill(std::transform(path.cbegin(),
                             path.cbegin() + std::min(std::size(path), std::size(dest)),
                             dest.begin(),
                             [] (const auto& vertex) { return vertex->first.point; }),
              dest.end(),
              std::nullopt);
}

} } // namespace simulacrum::ai

