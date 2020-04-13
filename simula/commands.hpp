#ifndef DEBUG_COMMANDS_HPP
#define DEBUG_COMMANDS_HPP

#include <string>
#include <vector>

struct bsp_export_options {
    char const* path_prefix = nullptr;

    struct surface_options {
        bool all       = true;
        bool walkable  = false;
        bool two_sided = false;
        bool invisible = false;
        bool climbable = false;
        bool breakable = false;
    } surface;

    struct nav_options {
        bool all       = true;
        bool nodes     = false;
        bool edges     = false;
    } nav;
};

bool ExportBSP(std::vector<std::string> const& args);

bool ExportBSP(bsp_export_options const& options);

bool ExportCBSPGraph(char const* filename);

bool BuildNavGraph();

#endif // DEBUG_COMMANDS_HPP
