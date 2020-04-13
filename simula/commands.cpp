#include "commands.hpp"

#include <cmath>

#include <string>
#include <fstream>

#include <sentinel/tag.hpp>
#include <sentinel/globals.hpp>
#include <sentinel/tags/structure_bsp.hpp>

#include "program_state.hpp"
#include "cbspgraph.hpp"

bool ExportBSP(bsp_export_options const& options) {
    using namespace sentinel;
    using namespace sentinel::tags;

    auto const *bsp_index  = sentinel__Globals_GetBSPIndex();
    auto const *map_globals = sentinel__Globals_GetMapGlobals();
    if (*bsp_index < 0 || !map_globals->structure_bsp) {
        printf("no loaded BSP\n");
        return false;
    }

    structure_bsp const *sbsp = map_globals->structure_bsp;

    printf("exporting structure bsp:%d (%p) with prefix \"%s\"\n",
           (int)*bsp_index, (void*)sbsp, options.path_prefix);

    for (int cbsp_index = 0; cbsp_index < sbsp->collision_bsps.count; ++cbsp_index) {
        collision_bsp const& cbsp = sbsp->collision_bsps[cbsp_index];
        std::string const path = std::string(options.path_prefix) + "." + std::to_string(cbsp_index) + ".obj";
        printf("exporting collision bsp:%d (%p) to \"%s\"\n",
               cbsp_index, (void*)&cbsp, path.c_str());

        auto test_surface = [o = options.surface, &cbsp] (collision_bsp::surface const& surface) {
            constexpr real sqrt2_2 = std::sqrt(2.0) / 2.0;

            real3 const& plane_normal = cbsp.planes[surface.plane_index()].normal;
            bool const is_reversed = surface.is_reversed();
            bool const is_walkable = (is_reversed ? -plane_normal[2] : plane_normal[2]) >= sqrt2_2;

            return o.all
                || (o.walkable  && is_walkable)
                || (o.two_sided && surface.test_flags(surface.TWO_SIDED))
                || (o.invisible && surface.test_flags(surface.INVISIBLE))
                || (o.climbable && surface.test_flags(surface.CLIMBABLE))
                || (o.breakable && surface.test_flags(surface.BREAKABLE));
        };

        std::ofstream o(path, std::ios_base::out | std::ios_base::trunc);

        if (!o.is_open()) {
            printf("could not open \"%s\" for writing\n", path.c_str());
            return false;
        }

        o << "o map_mesh" << std::endl;
        for (auto&& vertex : cbsp.vertices)
            o << "v " << vertex.point[0] << " " << vertex.point[1] << " " << vertex.point[2] << std::endl;
        printf("exported %d vertices\n", cbsp.vertices.count);

        // export surfaces
        int exported_surfaces = 0;
        for (int32 surface_index = 0; surface_index < cbsp.surfaces.count; ++surface_index) {
            auto&& surface = cbsp.surfaces[surface_index];

            if (!test_surface(surface))
                continue;

            int32 const first_edge = surface.first_edge;
            int32 edge_index = first_edge;
            o << "f";
            do {
                if (edge_index < 0) {
                    printf("edge has negative index: %d\n", edge_index);
                    return false;
                }

                auto&& edge = cbsp.edges[edge_index];
                o << " " << (edge.start_vertex(surface_index) + 1);
                edge_index = edge.next_edge(surface_index);
            } while(edge_index != first_edge);
            o << std::endl;
            ++exported_surfaces;
        }
        printf("exported %d surfaces\n", exported_surfaces);

        if (options.nav.all || options.nav.nodes) {
            o << "o navigation_nodes" << std::endl;
            for (auto&& node : simula::program_state.nav_graph.nodes) {
                constexpr real SCALE = 0.1;
                constexpr real3 DIAMOND_VERTICES[] = {
                    {SCALE, 0, 0},
                    {0, SCALE, 0},
                    {0, 0, SCALE},
                    {-SCALE, 0, 0},
                    {0, -SCALE, 0},
                    {0, 0, -SCALE},
                };

                constexpr char const* DIAMOND_SHAPE =
                    "f -6 -5 -4\n"
                    "f -6 -4 -2\n"
                    "f -6 -2 -1\n"
                    "f -6 -1 -5\n"
                    "f -3 -5 -1\n"
                    "f -3 -1 -2\n"
                    "f -3 -4 -5\n"
                    "f -3 -2 -4\n";

                for (real3 const& v : DIAMOND_VERTICES)
                    o << "v " << v[0] + node.world.point[0] << " " << v[1] + node.world.point[1] << " " << (v[2] + SCALE) + node.world.point[2] << std::endl;

                o << DIAMOND_SHAPE << std::endl;
            }
            printf("exported %u navigation nodes\n", simula::program_state.nav_graph.nodes.size());
        }

        if (options.nav.all || options.nav.edges) {
            o << "o navigation_edges" << std::endl;
            {
                //constexpr float BUMP_AMOUNT = 0.1;
                simula::nav::graph const& nav_graph = simula::program_state.nav_graph;
                constexpr float BUMP = 0.1;

                for (std::size_t node_index = 0; node_index < nav_graph.nodes.size(); ++node_index) {
                    real3 node_point = nav_graph.nodes[node_index].world.point;
                    for (auto const neighbor_index : nav_graph.neighbors_of(node_index)) {
                        real3 const& neighbor_point = nav_graph.nodes[neighbor_index].world.point;
                        //o << "# " << nav_graph.nodes[node_index].world.index << " -> " << nav_graph.nodes[neighbor_index].world.index << std::endl;
                        o << "v " << node_point[0] << " " << node_point[1] << " " << node_point[2] + BUMP << std::endl;
                        o << "v " << neighbor_point[0] << " " << neighbor_point[1] << " " << neighbor_point[2] + BUMP << std::endl;
                        o << "f -2 -1" << std::endl;
                    }
                }
            }
            printf("exported %u navigation edges\n", simula::program_state.nav_graph.neighbors.size());
        }
    }

    return true;
}

bool ExportBSP(std::vector<std::string> const& args) {
    // usage: <command> <path-prefix> [surface-options:walkable|climbable|breakable|two_sided]...

    if (args.size() < 2) {
        printf("usage: %s <path-prefix> [surface-options:walkable|climbable|breakable|two_sided]...\n", args[0].c_str());
        return false;
    }

    bsp_export_options options;
    options.path_prefix = args[1].c_str();
    options.surface.all = args.size() == 2;

    using surface_option_ptr = bool bsp_export_options::surface_options::*;
    struct surface_option_reflexive {
        char const*         name;
        surface_option_ptr  ptr;
    };

    constexpr surface_option_reflexive surface_reflexives[] = {
        {"walkable",  &bsp_export_options::surface_options::walkable},
        {"two_sided", &bsp_export_options::surface_options::two_sided},
        {"invisible", &bsp_export_options::surface_options::invisible},
        {"climbable", &bsp_export_options::surface_options::climbable},
        {"breakable", &bsp_export_options::surface_options::breakable}
    };

    for (typename std::decay_t<decltype(args)>::size_type i = 2; i < args.size(); ++i) {
        std::string const& str = args[i];

        for (auto&& desc : surface_reflexives) {
            if (str == desc.name)
                options.surface.*desc.ptr = true;
        }
    }

    return ExportBSP(options);
}

bool BuildNavGraph() {
    return simula::program_state.nav_graph.build();
}

bool ExportCBSPGraph(char const* filename) {
    using simula::nav::CBSPGraph;

    std::ofstream file(filename);
    if (!file.is_open())
        return false;

    sentinel::globals::map_globals const* map_globals = sentinel__Globals_GetMapGlobals();
    sentinel::tags::collision_bsp const* collision_bsp = map_globals->collision_bsp0;

    auto filter_surface = [collision_bsp] (long const index) -> bool {
        constexpr float sqrt2_2 = std::sqrt(2.0) / 2.0;
        auto const& surface = collision_bsp->surfaces[index];
        sentinel::real3 const& plane_normal = collision_bsp->planes[surface.plane_index()].normal;

        return surface.test_flags(surface.CLIMBABLE)
            || (surface.is_reversed() ? plane_normal[2] <= -sqrt2_2: plane_normal[2] >= sqrt2_2);
    };

    auto filter = [collision_bsp, &filter_surface] (simula::nav::CBSPElement const& element) -> bool {
        switch(element.type) {
        case element.TYPE::EDGE: // pass an edge if and only if both surface are walkable
            return filter_surface(element.as_edge(*collision_bsp).surfaces[0]) && filter_surface(element.as_edge(*collision_bsp).surfaces[1]);
        case element.TYPE::SURFACE:
            return false; //filter_surface(element.index);
        default:
            return false;
        }
    };

    CBSPGraph graph;
    printf("building cbspgraph for collision bsp @ %p\n", collision_bsp);
    if (graph.build(collision_bsp, true, filter))
        printf("cbspgraph built successfully\n");

    return graph.export_as_obj(file);
}
