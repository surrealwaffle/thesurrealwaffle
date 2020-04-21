
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "main.h"

#include <sentutil/all.hpp>
#include "bot_ai.hpp"
#include "bot_control.hpp"
#include "game_context.hpp"

#ifdef OLD
bool dump_bsp_model(const char* filename)
{
    if (!filename)
        return false;

    using namespace simulacrum::utility;
    if (sentutil::globals::map_globals->collision_bsp == nullptr)
        return false;

    // open file for writing
    std::ofstream out(filename);
    if (!out)
        return false;

    interface_cbsp coll{*sentutil::globals::map_globals->collision_bsp};

    // dump vertices
    for (const auto& vertex : coll.vertices) {
        const auto& point = vertex.point();
        out << "v " << point[0] << " "
                    << point[1] << " "
                    << point[2] << "\n";
    }

    // dump surfaces
    for (const auto& surface : coll.surfaces) {
        out << "f";
        for (const auto& vertex : surface.vertices())
            out << " " << (vertex.index + 1);
        out << "\n";
    }

    return static_cast<bool>(out);
}

namespace {

bool aim_target_lock = false;
std::optional<sentinel::real3d> aim_target_position;
std::optional<sentinel::real3d> desired_position;

simulacrum::navigation_graph nav_graph;

}

bool dump_nav_graph(const char* filename)
{
    if (!filename)
        return false;

    std::ofstream out(filename);
    if (!out)
        return false;

    // build node indices to have a properly connected .obj
    std::unordered_map<simulacrum::navigation_graph::node_type,
                       std::size_t> node_index;

    for (const auto& node : nav_graph)
        node_index.insert({node.first, node_index.size()});

    // dump vertices
    for (const auto& node : nav_graph) {
        const auto& centroid = node.first.point;
        out << "v " << centroid[0] << " "
                    << centroid[1] << " "
                    << centroid[2] << "\n";
    }

    // dump lines
    for (const auto& [node, edges] : nav_graph) {
        for (const auto& edge : edges) {
            out << "l " << (node_index[node] + 1) << " "
                        << (node_index[edge.target->first] + 1) << "\n";
        }
    }

    return static_cast<bool>(out);
}

void find_nearest_nav(std::optional<bool> lock)
{
    if (sentutil::globals::map_globals->collision_bsp == nullptr)
        return;

    using simulacrum::utility::interface_cbsp;
    interface_cbsp coll{*sentutil::globals::map_globals->collision_bsp};
    const simulacrum::navigation_graph nav(coll);

    const sentinel::real3d camera = sentutil::globals::camera_globals->position;

    if (auto it = nav.nearest_node(camera)) {
        const auto& [node, edges] = *it.value();
        const auto target_point = node.point;
        aim_target_position = target_point;
        aim_target_lock = lock.value_or(false);
    } else {
        sentutil::console::cprint({1, 1, 0, 0}, "no nearest nav node");
    }
}

struct exception_goal_found { };

void controls_filter(sentinel::digital_controls_state&,
                     sentinel::analog_controls_state& analog,
                     [[maybe_unused]] float seconds,
                     [[maybe_unused]] long ticks)
{
    const sentinel::real3d camera = sentutil::globals::camera_globals->position;
    const sentinel::real3d forward = sentutil::globals::camera_globals->forward;

    const auto opt_camera_nav_vertex = nav_graph.nearest_node(camera);

    if (aim_target_position) {
        sentinel::real3d subframe_delta = {/*ZERO INITIALIZED*/};
        sentinel::real3d u00 = {};
        sentinel::real3d u01 = {};
        sentinel_Engine_ExtrapolateLocalUnitDelta(&subframe_delta, &u00, &u01, seconds);

        const sentinel::real3d extrapolated_camera = camera + subframe_delta;

        const sentinel::real3d desired_target = aim_target_position.value();

        if (!aim_target_lock)
            aim_target_position = std::nullopt;

        const sentinel::real3d delta = desired_target - extrapolated_camera;

        // returns pitch/yaw
        auto get_looking_angles = [] (const sentinel::real3d& dir) -> std::pair<float, float> {
            const sentinel::real2d xy = {dir[0], dir[1]};
            const sentinel::real   z  = dir[2];

            return {std::atan2(xy[1], xy[0]), std::atan2(z, norm(xy))};
        };

        const auto [current_yaw, current_pitch] = get_looking_angles(forward);
        const auto [target_yaw,  target_pitch]  = get_looking_angles(delta);

        constexpr float pi = 0x1.921FB6p1;
        constexpr float half_pi = pi / 2;
        constexpr float twice_pi = 2 * pi;

        if (target_pitch < -half_pi || target_pitch > half_pi) {
            sentutil::console::cprint({1.0, 1.0, 0, 0}, "insanity occurred");
            return;
        }

        auto sanitize_angle = [] (float angle) {
            while (angle > pi)
                angle -= twice_pi;

            while (angle < -pi)
                angle += twice_pi;

            return angle;
        };

        analog.turn_left = sanitize_angle(target_yaw - current_yaw);
        analog.turn_up   = sanitize_angle(target_pitch - current_pitch);
    }

    if (desired_position) {
        auto unit_id = sentutil::globals::local_player_globals->players[0].unit;
            if (!unit_id)
                return;
        sentinel::unit& unit = *unit_id;

        const sentinel::real3d here   = unit.object.position; // not accurate for vehicles
        const sentinel::real3d target = desired_position.value();
        auto opt_nav_vertex = nav_graph.nearest_node(target);
        const bool close_enough = norm(target - here) <= 0.2f;

        if (close_enough || !opt_nav_vertex || !opt_camera_nav_vertex) {
            if (close_enough)
                sentutil::console::cprint({1, 0, 1, 0}, "destination reached");

            desired_position = std::nullopt;
            return;
        }

        using Node = simulacrum::navigation_graph_node;
        auto print_node_to_console = [] (const Node& node, const char* name) {
            sentutil::console::cprintf("node<%s>: %s [%ld]",
                                       name,
                                       node.cbsp_type == Node::type_edge ? "edge"
                                                                         : "surface",
                                       node.cbsp_index);
        };

        const Node& start = opt_camera_nav_vertex.value()->first;
        const Node& goal = opt_nav_vertex.value()->first;
        auto heuristic = [point = goal.point] (const Node& node) {
            return norm(node.point - point);
        };

        print_node_to_console(start, "start");
        print_node_to_console(goal, "goal");

        std::unordered_map<Node, Node>  predecessor_map;
        auto visitor = [&predecessor_map, goal] (const Node& node, const Node& predecessor) {
            predecessor_map.insert_or_assign(node, predecessor);
            if (node == goal)
                throw exception_goal_found();
        };

        bool path_found = false;
        predecessor_map.insert({start, start});
        try {
            std::unordered_map<Node, float> distance_map;
            simulacrum::astar_search(nav_graph.get_graph(),
                                     opt_camera_nav_vertex.value(),
                                     heuristic,
                                     0.0f,
                                     17320.5f,
                                     distance_map,
                                     [] (const auto& edge) { return edge->distance; },
                                     visitor);
        } catch (const exception_goal_found&) {
            path_found = true;
        }

        if (!path_found) {
            desired_position = std::nullopt;
            sentutil::console::cprint({1, 1, 0, 0}, "no path to target");
            return;
        } else if (start == goal) {
            desired_position = std::nullopt;
            sentutil::console::cprint({1, 0, 1, 0}, "destination reached");
            return;
        }

        try {
            Node next_node_to_goal = [start, goal, &predecessor_map] {
                Node node = goal;
                for (; predecessor_map.at(node) != start; node = predecessor_map.at(node)) {
                    /* DO NOTHING */
                }
                return node;
            }();

            const sentinel::real3d delta = next_node_to_goal.point - here;
            const sentinel::real2d delta2d = sentinel::real2d{delta[0], delta[1]};

            const sentinel::matrix2d body_to_world = {
                sentinel::real2d{unit.unit.body_forward[0], unit.unit.body_forward[1]},
                sentinel::real2d{-unit.unit.body_forward[1], unit.unit.body_forward[0]}
            };

            const sentinel::matrix2d world_to_body = transpose(body_to_world);

            const sentinel::real2d next_node_relative = world_to_body * delta2d;

            analog.move_forward = next_node_relative[0];
            analog.move_left = next_node_relative[1];
        } catch (const std::exception& e) {
            aim_target_lock = false;
            aim_target_position = std::nullopt;
            desired_position = std::nullopt;
            sentutil::console::cprintf({1, 1, 0, 0}, "fatal: exception when reconstructing path to to goal (%s)", e.what());
            return;
        }

    }
}

bool go_to_point()
{
    const auto unit = sentutil::globals::local_player_globals->players[0].unit;
    const auto camera = sentutil::globals::camera_globals->position;
    const auto forward = sentutil::globals::camera_globals->forward;

    std::optional raycast = sentutil::raycast::cast_projectile_ray(
        camera,
        8660.0f * forward,
        unit);

    desired_position = raycast ? std::make_optional(raycast.value().terminal)
                               : std::nullopt;

    return static_cast<bool>(desired_position);
}

void on_map_instantiate()
{
    desired_position = std::nullopt;
    nav_graph = simulacrum::navigation_graph(simulacrum::this_collision_bsp);
}

void display_value(std::string_view name)
{
    auto unit_id = sentutil::globals::local_player_globals->players[0].unit;
    if (!unit_id)
        return;

    using sentutil::console::cprintf;
    sentinel::unit& unit = *unit_id;
    sentinel::real3d* pValue = nullptr;

    if (name == "object.position")     pValue = &unit.object.position;
    else if (name == "object.forward") pValue = &unit.object.forward;
    else if (name == "object.up")      pValue = &unit.object.up;
    else if (name == "unit.body_forward") pValue = &unit.unit.body_forward;
    else if (name == "unit.aim_forward")  pValue = &unit.unit.aim_forward;

    if (pValue) {
        sentinel::real3d& value = *pValue;
        cprintf("%p: (%f, %f, %f)", &value, value[0], value[1], value[2]);
    }
}
#endif // OLD

namespace {

bool        simulacrum_enabled = false;
std::string current_cache_name;

void load_map_cache(std::string_view cache_name);

void instantiate_map();

void controls_filter(sentinel::digital_controls_state& digital,
                     sentinel::analog_controls_state&  analog,
                     float seconds,
                     long  ticks);

bool Load()
{
    using sentutil::controls::install_controls_filter;
    using sentutil::events::install_load_map_cache_callback;
    using sentutil::events::install_map_instantiation_callback;
    using sentutil::script::install_script_function;

    return
        install_controls_filter(controls_filter) &&
        install_load_map_cache_callback(load_map_cache) &&
        install_map_instantiation_callback(instantiate_map) &&
        install_script_function<"simulacrum_reset">(
            +[] { simulacrum::ai::reset(); simulacrum::control::reset(); },
            "hard reset for the simulacrum AI and control structures",
            "") &&
        install_script_function<"simulacrum_enabled">(
            +[] (bool b) { simulacrum_enabled = b; },
            "enables or disables the simulacrum AI and control"
            "<bool>") &&
        simulacrum::game_context.load() &&
        simulacrum::ai::load() &&
        simulacrum::control::load(); /* &&
        // OLD: phase these out
        install_script_function<"dump_bsp_model">(dump_bsp_model,
                                                  "dumps the bsp model",
                                                  "<string: filename>") &&
        install_script_function<"dump_nav_graph">(dump_nav_graph,
                                                  "dumps the nav graph"
                                                  "<string: filename>") &&
        install_script_function<"find_nearest_nav">(find_nearest_nav,
                                                    "points to the nearest nav node",
                                                    "[bool: lock = false]") &&
        install_script_function<"go_to_point">(go_to_point,
                                               "",
                                               "") &&
        install_script_function<"display_value">(display_value,
                                                 "",
                                                 "") &&
        sentutil::controls::install_controls_filter(controls_filter) &&
        sentutil::events::install_map_instantiation_callback(on_map_instantiate);
        */
}

} // namespace (anonymous)



extern "C" DLL_EXPORT BOOL APIENTRY DllMain([[maybe_unused]] HINSTANCE hinstDLL,
                                            DWORD fdwReason,
                                            [[maybe_unused]] LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            if (!Load())
                return FALSE;
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}

namespace {

void load_map_cache(std::string_view cache_name)
{
    current_cache_name = cache_name;
}

void instantiate_map()
{
    simulacrum::ai::recalculate_navigation(std::string_view(current_cache_name));
    simulacrum::control::reset();
}

void controls_filter(sentinel::digital_controls_state& digital,
                     sentinel::analog_controls_state&  analog,
                     float seconds,
                     long  ticks)
{
    /* not needed if you are 1337 haxorz
    bool* tabbed_freeze_game = reinterpret_cast<bool*>(
        sentutil::globals::game_edition == sentinel::GameEdition::combat_evolved
            ? 0x721E8C : 0x6BD15C);
    bool* tabbed_disable_dyna_sound = reinterpret_cast<bool*>(
        sentutil::globals::game_edition == sentinel::GameEdition::combat_evolved
            ? 0x725202 : 0x6E13D8);
    bool* tabbed_disable_cont_sound = reinterpret_cast<bool*>(
        sentutil::globals::game_edition == sentinel::GameEdition::combat_evolved
            ? 0x746118 : 0x6C04C2);

    *tabbed_freeze_game = *tabbed_disable_dyna_sound = *tabbed_disable_cont_sound = false;
    */

    simulacrum::game_context.preupdate(ticks);
    if (simulacrum_enabled) {
        simulacrum::ai::update(seconds, ticks);
        simulacrum::control::update(digital, analog, seconds, ticks);
    }
    simulacrum::game_context.postupdate(digital);
}

} // namespace (anonymous)
