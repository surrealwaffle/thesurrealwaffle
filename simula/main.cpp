#include "main.h"

#include "commands.hpp"
#include "filters.hpp"
#include "program_state.hpp"

#include <sentinel/console.hpp>
#include <sentinel/control.hpp>
#include <sentinel/events.hpp>
#include <sentinel/chat.hpp>

bool Load() {
    simula::program_state.reset();
    return sentinel::InstallConsoleCommand("sim_export_bsp", [] (auto const& a) { return ExportBSP(a); })
        && sentinel::InstallConsoleCommand("sim_build_nav", [] (auto const&) { return BuildNavGraph(); })
        && sentinel::InstallConsoleCommand("sim_export_cbsp_graph", [] (auto const& args) { return args.size() == 2 && ExportCBSPGraph(args[1].c_str()); })
        && sentinel::InstallMapLoadFilter(simula::filters::OnMapLoad)
        && sentinel::InstallControlFilter(simula::filters::OnControlsUpdate)
        && sentinel::InstallChatReceiveFilter(simula::filters::OnChatReceive)
        && sentinel::InstallConsoleCommand("sim_enable", [] (auto const&) { simula::program_state.persistent.enabled = true; return true; } )
        && sentinel::InstallConsoleCommand("sim_disable", [] (auto const&) { simula::program_state.persistent.enabled = false; return true; } )
        && sentinel::InstallConsoleCommand("sim_reset", [] (auto const&) { simula::program_state.reset(); return true; } );
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE, DWORD fdwReason, LPVOID)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            return Load() ? TRUE : FALSE;
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
