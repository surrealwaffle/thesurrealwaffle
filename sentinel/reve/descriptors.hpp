#pragma once

namespace reve {

bool Init();

void Debug();

} // reve

/* OLD STUFF

    // modify on the fly to change the name the user connects to servers with
    patch_descriptor const player_name_desc {
        __FILE__ ": Globals::PlayerName",
        bytes{0xB9, 0xFF, 0x07, 0x00, 0x00,   // MOV ECX, 0x07FF
              0xBE}, read{&lpwszPlayerName},  // MOV ESI, offset:<Globals::PlayerName>
        bytes{0x8D, 0x7C, 0x24, 0x48}         // LEA EDI,[LOCAL.2304]
    };

    // self-explanatory, but obscure usage
    patch_descriptor const is_window_tabbed_desc {
        __FILE__ ": Globals::IsWindowTabbed",
        bytes{0xA3, -1u, -1u, -1u, -1u,             // MOV DWORD PTR DS:[*], EAX
              0xC6, 0x05, -1u, -1u, -1u, -1u, 0x01, // MOV BYTE PTR DS:[*], 01
              0x38, 0x1D}, read{&lpIsWindowTabbed} // CMP BYTE PTR DS:[Globals::IsWindowTabbed], BL
    };

    // NOT IN USE, also in odbg as Engine::UpdateTick(dwTickIndex)
    // See also Engine::UpdateGame(float dt_seconds)
    patch_descriptor const update_world_ticks_desc = {
        __FILE__ ", line " SENTINEL_STRINGIZE(__LINE__) ": Engine::UpdateWorldTicks",
        read{(void**)&::update_world_ticks_proc},
        bytes{0x51, // PUSH ECX
              0x53, // PUSH EBX
              0x68, 0xFF, 0xFF, 0x0F, 0x00, // PUSH 0x000FFFFF
              0x68, 0x1F, 0x00, 0x09, 0x00} // PUSH 0x0009001F
    };

*/

