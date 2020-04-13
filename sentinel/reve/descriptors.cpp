#include "descriptors.hpp"

#include <iostream>    // std::cout
#include <functional>  // std::ref
#include <string_view> // std::string_view
#include <tuple>       // std::tuple

#include <detours/detours.hpp>

#include "chat.hpp"
#include "controls.hpp"
#include "console.hpp"
#include "engine.hpp"
#include "globals.hpp"
#include "init.hpp"
#include "memory.hpp"
#include "object.hpp"
#include "raycast.hpp"
#include "script.hpp"
#include "sound.hpp"
#include "table.hpp"

namespace reve::descriptors {

using namespace detours::descriptors;
using std::ref;

/*
$ ==> |.  C64424 10 FF   MOV BYTE PTR SS:[LOCAL.450],0FF
$+5   |.  895424 14      MOV DWORD PTR SS:[LOCAL.449],EDX
$+9   |.  E8 E6150400    CALL Network::DecodeMessage
$+E   |.  84C0           TEST AL,AL
*/
static const descriptor_sequence chat_DecodeChatUpdate {
    bytes{0xC6, 0x44, 0x24, 0x10, 0xFF,
          0x89, 0x54, 0x24, 0x14},
    detour{detour_call,
           chat::hook_DecodeChatUpdate, ref(chat::proc_DecodeChatUpdate)},
    bytes{0x84, 0xC0}
}; // chat_DecodeChatUpdate

/*
$ ==> |.  66:C7447C 1C 0000  MOV WORD PTR SS:[EDI*2+ESP+1C],0
$+7   |.  E8 90000000        CALL chat::SendChatToServer
*/
static const descriptor_sequence chat_SendChatToServer {
    bytes{0x66, 0xC7, 0x44, 0x7C, 0x1C, 0x00, 0x00},
    read_call_rel32{ref(chat::proc_SendChatToServer)}
}; // chat_SendChatToServer

/*
$ ==> |.  50                        PUSH EAX
$+1   |.  66:C78424 22080000 0000   MOV WORD PTR SS:[LOCAL.0+2],0
$+B   |.  E8 E2AF0400               CALL chat::chat_EnqueueChatEntry
$+10  |.  83C4 04                   ADD ESP,4
*/
static const descriptor_sequence chat_EnqueueChatEntry {
    bytes{0x50,
          0x66, 0xC7, 0x84, 0x24, 0x22, 0x08, 0x00, 0x00, 0x00, 0x00},
    read_call_rel32{ref(chat::proc_EnqueueChatEntry)}
}; // chat_EnqueueChatEntry

/*
$ ==> |.  57             PUSH EDI
$+1   |.  56             PUSH ESI
$+2   |.  51             PUSH ECX
$+3   |.  52             PUSH EDX
$+4   |.  68 CC096700    PUSH OFFSET 006709CC  ; Format = "%-15s %-20s %02d:%02d:%02d"
$+9   |.  E8 B9D9F5FF    CALL console::TerminalPrintf
*/
static const descriptor_sequence console_TerminalPrintf {
    bytes{0x57,
          0x56,
          0x51,
          0x52,
          0x68, -1, -1, -1, -1},
    read_call_rel32{ref(console::proc_TerminalPrintf)}
}; // console_TerminalPrintf

/*
$ ==> |.  381D 20706B00  |CMP BYTE PTR DS:[ConsoleGlobals.is_open],BL
$+6   |.  75 05          |JNE SHORT 004C7E27 ; jumps over call to TerminalUpdate below
$+8   |.  E8 B9E8FCFF    |CALL console::TerminalUpdate
$+D   |>  E8 14EFFCFF    |CALL 00496D40
*/
static const descriptor_sequence console_TerminalUpdate {
    bytes{0x38, 0x1D}, read_pointer{ref(globals::ptr_ConsoleGlobals)},
    bytes{0x75, 0x05},
    detour{detour_call,
           console::hook_TerminalUpdate, ref(console::proc_TerminalUpdate)},
    bytes{0xE8}
}; // console_TerminalUpdate

/*
$ ==> |.  F3:AB          REP STOS DWORD PTR ES:[EDI]
$+2   |.  FFD6           CALL ESI
$+4   |>  881D FC2E6B00  MOV BYTE PTR DS:[Terminal.initialized],BL
*/
static const descriptor_sequence console_Terminal {
    bytes{0xF3, 0xAB,
          0xFF, 0xD6,
          0x88, 0x1D}, read_pointer{ref(console::ptr_Terminal)}
}; // console_Terminal

/*
$ ==> |.  52             PUSH EDX
$+1   |.  51             PUSH ECX
$+2   |.  50             PUSH EAX
$+3   |.  897424 2C      MOV DWORD PTR SS:[LOCAL.16],ESI
$+7   |.  E8 6AF5FFFF    CALL controls::GetUserActions
*/
static const descriptor_sequence controls_GetUserActions {
    bytes{0x52,
          0x51,
          0x50,
          0x89, 0x74, 0x24, 0x2C},
    detour{detour_call,
           controls::hook_GetUserActions, ref(controls::proc_GetUserActions)}
}; // controls_GetUserActions

/*
$ ==> |.  8935 C47E6F00    |MOV DWORD PTR DS:[*],ESI
$+6   |.  E8 359CFAFF      |CALL controls::ProcessUserControls
$+B   |.  0FBF05 20977100  |MOVSX EAX,WORD PTR DS:[connection_type]
*/
static const descriptor_sequence controls_ProcessUserControls {
    bytes{0x89, 0x35, -1, -1, -1, -1},
    detour{detour_call,
           controls::hook_ProcessUserControls,
           ref(controls::proc_ProcessUserControls)},
    bytes{0x0F, 0xBF, 0x05}
}; // controls_ProcessUserControls

/*
$ ==> |.  BE 98247100  MOV ESI,OFFSET controls::ControlsState
$+5   |.  8D7C24 54    LEA EDI,[LOCAL.9]
*/
static const descriptor_sequence controls_ControlsState {
    bytes{0xBE}, read_pointer{ref(controls::ptr_ControlsState)},
    bytes{0x8D, 0x7C, 0x24, 0x54}
}; // controls_ControlsState

/*
$ ==>  |.  FF0D CC9C7100 DEC DWORD PTR DS:[719CCC]
$+6    |.  E8 12470000   CALL engine::UpdateNetgameFlags
$+B    |.  E8 7DE90200   CALL 0048A1A0 ; script threads
$+10   |.  E8 68F2FEFF   CALL 0044AA90 ; recorded animations
$+15   |.  E8 63960900   CALL engine::UpdateObjects
$+1A   |.  E8 6E910100   CALL 004749A0
$+1F   |.  E8 59E10400   CALL 004A9990
$+24   |.  E8 F4AEFFFF   CALL 00456730
*/
static const descriptor_sequence engine_UpdateMapEntities {
    bytes{0xFF, 0x0D, -1, -1, -1, -1},
    read_call_rel32{ref(engine::proc_UpdateNetgameFlags)},
    bytes{0xE8, -1, -1, -1, -1,
          0xE8, -1, -1, -1, -1},
    read_call_rel32{ref(engine::proc_UpdateObjects)}
};

/*
$ ==>     |.  8D5424 10     LEA EDX,[LOCAL.8]
$+4       |.  52            PUSH EDX
$+5       |.  E8 49531100   CALL engine::ExtrapolateLocalUnitDelta
$+A       |.  83C4 10       ADD ESP,10
*/
static const descriptor_sequence engine_ExtrapolateLocalUnitDelta {
    bytes{0x8D, 0x54, 0x24, 0x10,
          0x52},
    read_call_rel32{ref(engine::proc_ExtrapolateLocalUnitDelta)},
    bytes{0x83, 0xC4, 0x10}
};

/*
$ ==>  |.  51           PUSH ECX
$+1    |.  8BC6         MOV EAX,ESI
$+3    |.  E8 741B0000  CALL GetBipedPositionUpdateFlags
$+8    |.  83C4 04      ADD ESP,4
$+B    |>  8D55 FC      LEA EDX,[LOCAL.1]
$+E    |.  52           PUSH EDX
$+F    |.  56           PUSH ESI
$+10   |.  E8 77330000  CALL engine::UpdateBipedPosition
*/
static const descriptor_sequence engine_UpdateBipedPosition {
    bytes{0x51,
          0x8B, 0xC6},
    read_call_rel32{ref(engine::proc_GetBipedUpdatePositionFlags)},
    bytes{0x83, 0xC4, 0x04,
          0x8D, 0x55, 0xFC,
          0x52, 0x56},
    read_call_rel32{ref(engine::proc_UpdateBipedPosition)}
};

/*
$ ==>  |.  8946 18       MOV DWORD PTR DS:[ESI+18],EAX
$+3    |.  8935 6C1D6F00 MOV DWORD PTR DS:[pGameTimeGlobals],ESI
$+9    |.  8946 1C       MOV DWORD PTR DS:[ESI+1C],EAX
$+C    |.  5E            POP ESI
$+D    |.  59            POP ECX
*/
static const descriptor_sequence globals_GameTimeGlobals {
    bytes{0x89, 0x46, 0x18,
          0x89, 0x35}, read_pointer{ref(globals::ptr_pGameTimeGlobals)},
    bytes{0x89, 0x46, 0x1C,
          0x5E,
          0x59}
}; // globals_GameTimeGlobals

/*
$ ==>  |.  83C4 28       ADD ESP,28
$+3    |.  8935 5C146B00 MOV DWORD PTR DS:[pLocalPlayerGlobals],ESI
$+9    |.  5E            POP ESI
$+A    |.  5B            POP EBX
$+B    |.  59            POP ECX
*/
static const descriptor_sequence globals_LocalPlayerGlobals {
    bytes{0x83, 0xC4, 0x28,
          0x89, 0x35}, read_pointer{ref(globals::ptr_pLocalPlayerGlobals)},
    bytes{0x5E,
          0x5B,
          0x59}
}; // globals_LocalPlayerGlobals

/*
CPU Disasm
Addres>Hex dump          Command
$ ==>  |.  68 00300000   PUSH 3000
$+5    |.  A3 4CC56A00   MOV DWORD PTR DS:[pTagsArrayHeader],EAX
$+A    |.  68 0000B401   PUSH 1B40000
*/
static const descriptor_sequence globals_TagsArrayHeader {
    bytes{0x68, 0x00, 0x30, 0x00, 0x00,
          0xA3}, read_pointer{ref(globals::ptr_pTagsArrayHeader)},
    bytes{0x68, 0x00, 0x00, 0xB4, 0x01}
}; // globals_TagsArrayHeader

/*
$ ==>  |.  8B0D C82D6E00  MOV ECX,DWORD PTR DS:[AllocatorGlobals.base]
$+6    |.  6A 04          PUSH 4
$+8    |.  8D5424 0C      LEA EDX,[LOCAL.0]
*/
static const descriptor_sequence globals_AllocatorGlobals {
    bytes{0x8B, 0x0D}, read_pointer{ref(globals::ptr_AllocatorGlobals)},
    bytes{0x6A, 0x04,
          0x8D, 0x54, 0x24, 0x0C}
}; // globals_AllocatorGlobals

/*
$ ==>  |.  D84424 04      FADD DWORD PTR SS:[LOCAL.8]
$+4    |.  D91D D0C66A00  FSTP DWORD PTR DS:[CameraGlobals.position]
*/
static const descriptor_sequence globals_CameraGlobals {
    bytes{0xD8, 0x44, 0x24, 0x04,
          0xD9, 0x1D}, read_pointer{ref(globals::ptr_CameraGlobals)}
}; // globals_CameraGlobals

/*
$ ==>  |> /83C1 0C        /ADD ECX,0C
$+3    |. |46             |INC ESI
$+4    |. |81F9 58386B00  |CMP ECX,OFFSET ChatGlobals.is_open
*/
static const descriptor_sequence globals_ChatGlobals {
    bytes{0x83, 0xC1, 0x0C,
          0x46,
          0x81, 0xF9}, read_pointer{ref(globals::ptr_ChatGlobals)}
}; // globals_ChatGlobals

/*
$ ==>  |.  53             |PUSH EBX
$+1    |.  68 E8030000    |PUSH 3E8
$+6    |.  51             |PUSH ECX
$+7    |.  50             |PUSH EAX
$+8    |.  A3 00977100    |MOV DWORD PTR DS:[MachineGlobals.now.LowPart], EAX
*/
static const descriptor_sequence globals_MachineGlobals {
    bytes{0x53,
          0x68, 0xE8, 0x03, 0x00, 0x00,
          0x51,
          0x50,
          0xA3}, read_pointer{ref(globals::ptr_MachineGlobals)}
}; // globals_MachineGlobals

/*
$ ==>  |> /DB05 886F7400  /FILD DWORD PTR DS:[MapGlobals.frames_rendered]
$+6    |. |83C2 04        |ADD EDX,4
$+9    |. |D84C17 FC      |FMUL DWORD PTR DS:[EDX+EDI-4]
*/
static const descriptor_sequence globals_MapGlobals {
    bytes{0xDB, 0x05}, read_pointer{ref(globals::ptr_MapGlobals)},
    bytes{0x83, 0xC2, 0x04,
          0xD8, 0x4C, 0x17, 0xFC}
}; // globals_MapGlobals

/*
$ ==>  |> /D94424 10      FLD DWORD PTR SS:[LOCAL.0]
$+4    |. |5F             POP EDI
$+5    |. |D805 A4527200  FADD DWORD PTR DS:[RuntimeSoundGlobals.current_volume]
*/
static const descriptor_sequence globals_RuntimeSoundGlobals {
    bytes{0xD9, 0x44, 0x24, 0x10,
          0x5F,
          0xD8, 0x05}, read_pointer{ref(globals::ptr_RuntimeSoundGlobals)}
}; // globals_RuntimeSoundGlobals

/*
$ ==>  |> \8B15 C0356E00  MOV EDX,DWORD PTR DS:[CommandLineArgs.argv]
$+6    |.  52             PUSH EDX
$+7    |.  8D4424 5C      LEA EAX,[LOCAL.64]
$+B    |.  50             PUSH EAX
*/
static const descriptor_sequence globals_CommandLineArgs {
    bytes{0x8B, 0x15}, read_pointer{ref(globals::ptr_CommandLineArgs)},
    bytes{0x52,
          0x8D, 0x44, 0x24, 0x5C,
          0x50}
}; // globals_CommandLineArgs

/*
$ ==>  |.  52             PUSH EDX
$+1    |.  8D75 D0        LEA ESI,[EBP-30]
$+4    |.  B8 940F6700    MOV EAX,OFFSET EditionString
*/
static const descriptor_sequence globals_EditionString {
    bytes{0x52,
          0x8D, 0x75, 0xD0,
          0xB8}, read_pointer{ref(globals::ptr_EditionString)}
}; // globals_EditionString

/*
CPU Disasm
Addres>Hex dump           Command                                                  Comments
$ ==>  |.  891D AC856B00  MOV DWORD PTR DS:[6B85AC],EBX
$+6    |.  891D B0856B00  MOV DWORD PTR DS:[6B85B0],EBX
$+C    |.  E8 74ECFFFF    CALL init::ProcessInitConfig
$+11   |.  E8 AF260000    CALL 004C9DD0 ; initializes UI map/runtime globals
$+16   |.  E8 0A280000    CALL init::InitMachineTimeGlobals
$+1B   |.  E8 55250000    CALL init::ProcessConnectArgs
*/
static const descriptor_sequence init_ProcessStartup {
    bytes{0x89, 0x1D, -1, -1, -1, -1,
          0x89, 0x1D, -1, -1, -1, -1},
    detour{detour_call,
           init::hook_ProcessInitConfig, ref(init::proc_ProcessInitConfig)},
    bytes{0xE8, -1, -1, -1, -1,
          0xE8, -1, -1, -1, -1},
    detour{detour_call,
           init::hook_ProcessConnectArgs, ref(init::proc_ProcessConnectArgs)}
}; // init_ProcessStartup

/*
$ ==>  |.  8BC5           MOV EAX,EBP
$+2    |.  F3:A5          REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS:[ESI]
$+4    |.  E8 7A500700    CALL Init::LoadMapCache
$+9    |.  33DB           XOR EBX,EBX
$+B    |.  84C0           TEST AL,AL
*/
static const descriptor_sequence init_LoadMapCacheSP {
    bytes{0x8B, 0xC5,
          0xF3, 0xA5},
    detour{detour_call, init::hook_LoadMapCache, ref(init::proc_LoadMapCache)},
    bytes{0x33, 0xDB,
          0x84, 0xC0}
}; // init_LoadMapCacheSP

/*
$ ==>  |.  8D7424 10      LEA ESI,[LOCAL.67]
$+4    |.  8D4424 1C      LEA EAX,[LOCAL.64]
$+8    |.  F3:A5          REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS:[ESI]
$+A    |.  E8 C3FE0500    CALL Init::LoadMapCache
$+F    |.  84C0           TEST AL,AL
*/
static const descriptor_sequence init_LoadMapCacheMP {
    bytes{0x8D, 0x74, 0x24, 0x10,
          0x8D, 0x44, 0x24, 0x1C,
          0xF3, 0xA5},
    detour{detour_call, init::hook_LoadMapCache},
    bytes{0x84, 0xC0}
}; // init_LoadMapCacheMP

/*
?? differs between PC and CE (AC on PC, EC on CE)
$ ==>  |> \889D ??030000  MOV BYTE PTR SS:[EBP+3**],BL
$+6    |.  E8 50C8F7FF    CALL Engine::InstantiateMap
*/
static const descriptor_sequence init_InstantiateMap {
    bytes{0x88, 0x9D, -1, 0x03, 0x00, 0x00},
    detour{detour_target, init::tramp_InstantiateMap, ref(init::proc_InstantiateMap)}
}; // init_InstantiateMap

/*
$ ==>     |.  8B35 BCA06300  MOV ESI,DWORD PTR DS:[<&KERNEL32.GlobalFree>]
$+6       |.  33DB           XOR EBX,EBX
$+8       |.  3BC3           CMP EAX,EBX
*/
static const descriptor_sequence memory_GlobalFreeImport {
    bytes{0x8B, 0x35}, read_pointer{ref(memory::import_proc_GlobalFree)},
    bytes{0x33, 0xDB,
          0x3B, 0xC3},

    perform_assignment_deref{ref(memory::proc_GlobalFree),
                             ref(memory::import_proc_GlobalFree)},
    assign_indirect{ref(memory::import_proc_GlobalFree), &memory::hook_GlobalFree}
}; // memory_GlobalFreeImport

/*
$ ==>     |> \6A 01            PUSH 1                  ; /Arg4 = 1
$+2       |.  8D9424 A4000000  LEA EDX,[LOCAL.27]      ; |
$+9       |.  52               PUSH EDX                ; |Arg3 => OFFSET LOCAL.27
$+A       |.  68 FC1F6700      PUSH OFFSET 00671FFC    ; |Arg2 = ASCII "left hand"
$+F       |.  53               PUSH EBX                ; |Arg1 => [ARG.1]
$+10      |.  E8 277DF8FF      CALL GetObjectMarkers
*/
static const descriptor_sequence object_GetObjectMarkers {
    bytes{0x6A, 0x01,
          0x8D, 0x94, 0x24, 0xA4, 0x00, 0x00, 0x00,
          0x52,
          0x68, -1, -1, -1, -1,
          0x53},
    read_call_rel32{ref(object::proc_GetObjectMarkers)}
}; // object_GetObjectMarkers

/*
$ ==>     |.  8B4410 34     MOV EAX,DWORD PTR DS:[EDX+EAX+34]
$+4       |>  8D7C24 30     LEA EDI,[LOCAL.22]
$+8       |.  8BC8          MOV ECX,EAX
$+A       |.  E8 5C680B00   CALL GetUnitCameraPosition
*/
static const descriptor_sequence object_GetUnitCameraPosition {
    bytes{0x8B, 0x44, 0x10, 0x34,
          0x8D, 0x7C, 0x24, 0x30,
          0x8B, 0xC8},
    read_call_rel32{ref(object::proc_GetUnitCameraPosition)}
}; // object_GetUnitCameraPosition

/*                               Comments
$ ==>     |.  8955 E0           MOV DWORD PTR SS:[LOCAL.8],EDX
$+3       |.  894D E8           MOV DWORD PTR SS:[LOCAL.6],ECX
$+6       |.  C745 F8 DCBF6900  MOV DWORD PTR SS:[LOCAL.2],OFFSET ObjectPrototypes
*/
static const descriptor_sequence object_ObjectPrototypes {
    bytes{0x89, 0x55, 0xE0,
          0x89, 0x4D, 0xE8,
          0xC7, 0x45, 0xF8}, read_pointer{ref(object::ptr_pObjectPrototypes)}
}; // object_ObjectPrototypes

/*
$ ==>     |.  D95C24 10    FSTP DWORD PTR SS:[LOCAL.0]   ; |
$+4       |.  51           PUSH ECX                      ; |Arg3 => OFFSET LOCAL.2
$+5       |.  50           PUSH EAX                      ; |Arg2 => ARG.EAX
$+6       |.  52           PUSH EDX                      ; |Arg1 => [ARG.1]
$+7       |.  E8 283E1000  CALL CastRay
*/
static const descriptor_sequence raycast_CastRay {
    bytes{0xD9, 0x5C, 0x24, 0x10,
          0x51,
          0x50,
          0x52},
    read_call_rel32{ref(raycast::proc_CastRay)}
}; // raycast_CastRay

/*
$ ==>     |.  8BF8             MOV EDI,EAX
$+2       |.  33F6             XOR ESI,ESI
$+4       |.  BB 588B6800      MOV EBX,OFFSET script::ScriptFunctionsArray
*/
static const descriptor_sequence script_ScriptFunctionsArray {
    bytes{0x8B, 0xF8,
          0x33, 0xF6,
          0xBB}, read_pointer{ref(script::ptr_ScriptFunctionsArray)}
};

/*
$ ==>   |.  BF 80936800   MOV EDI,OFFSET SymbolLookupTable
$+5     |.  BB 12000000   MOV EBX,12
*/
static const descriptor_sequence script_SymbolLookupProcedures {
    bytes{0xBF}, read_pointer{ref(script::ptr_SymbolLookupProcedures)},
    bytes{0xBB, 0x12, 0x00, 0x00, 0x00}
}; // script_SymbolLookupProcedures

/*
$ ==>   |.  3BC6              CMP EAX,ESI
$+2     |.  57                PUSH EDI
$+3     |.  66:8935 B0146B00  MOV WORD PTR DS:[SymbolLookupBuffer.candidates_count],SI
$+A     |.  66:890D A0146B00  MOV WORD PTR DS:[SymbolLookupBuffer.capacity],CX
*/
static const descriptor_sequence script_UserEvaluationBuffer {
    bytes{0x3B, 0xC6,
          0x57,
          0x66, 0x89, 0x35, -1, -1, -1, -1,
          0x66, 0x89, 0x0D}, read_pointer{ref(script::ptr_UserEvaluationBuffer)}
}; // script_UserEvaluationBuffer

/*
$ ==>   |> \57                 PUSH EDI
$+1     |.  C605 FD027100 01   MOV BYTE PTR DS:[*],1
$+8     |.  E8 52D8FBFF        CALL ProcessExpression
$+D     |.  83C4 04            ADD ESP,4
$+10    |.  5E                 POP ESI
*/
static const descriptor_sequence script_ProcessExpression {
    bytes{0x57,
          0xC6, 0x05, -1, -1, -1, -1, 0x01},
    read_call_rel32{ref(script::proc_ProcessExpression)},
    bytes{0x83, 0xC4, 0x04,
          0x5E}
}; // script_ProcessExpression

/*
$ ==>  |.  D95C24 10     FSTP DWORD PTR SS:[ARG.1]
$+4    |.  8B4424 10     MOV EAX,DWORD PTR SS:[ARG.1]
$+8    |.  E8 20BC0000   CALL hs::FunctionContextReturn
*/
static const descriptor_sequence script_FunctionContextReturn {
    bytes{0xD9, 0x5C, 0x24, 0x10,
          0x8B, 0x44, 0x24, 0x10},
    read_call_rel32{ref(script::proc_FunctionContextReturn)}
}; // script_FunctionContextReturn

/*
$ ==>  |.  6A 08        PUSH 8                               ; /value_type: 8 (long)
$+2    |.  57           PUSH EDI                             ; |script_node_id
$+3    |.  E8 86100000  CALL hs::parse_script_node_expected  ; returns zero iff fail
*/
static const descriptor_sequence script_ParseScriptNodeExpected {
    bytes{0x6A, 0x08,
          0x57},
    read_call_rel32{ref(script::proc_ParseScriptNodeExpected)},
    bytes{0x83, 0xC4, 0x08}
}; // script_ParseScriptNodeExpected

/*
$ ==>  |.  8B5424 28    MOV EDX,DWORD PTR SS:[ARG.2]
$+4    |.  8BD8         MOV EBX,EAX
$+6    |.  8BC6         MOV EAX,ESI
$+8    |.  E8 FB0B0000  CALL hs::ThreadPushEvalFrame
*/
static const descriptor_sequence script_PushEvalFrame {
    bytes{0x8D, 0x14, 0x80,
          0x8B, 0x44, 0x91, 0x08,
          0x8B, 0x54, 0x24, 0x28},
    read_call_rel32{ref(script::proc_PushEvalFrame)}
}; // script_PushEvalFrame

/*
$ ==>   |.  55                 PUSH EBP
$+1     |.  68 0C617400        PUSH OFFSET DirectSoundInterfaces.lpDirectSound
$+6     |.  55                 PUSH EBP
$+7     |.  FF15 70627400      CALL DWORD PTR DS:[pDirectSoundCreate8]
*/
static const descriptor_sequence sound_DirectSoundInterfaces {
    bytes{0x55,
          0x68}, read_pointer{ref(sound::ptr_DirectSoundInterfaces)},
    bytes{0x55,
          0xFF, 0x15}
};

/*
$ ==>  |> \0FBF15 28547200    |MOVSX EDX,WORD PTR DS:[SecondaryBuffers.count]
$+7    |.  47                 |INC EDI
*/
static const descriptor_sequence sound_SecondarySoundBuffers {
    bytes{0x0F, 0xBF, 0x15}, read_pointer{ref(sound::ptr_SecondarySoundBuffers)},
    bytes{0x47}
};

/* not unique, but all matches call the function
$ ==>  |> \8BD6         |MOV EDX,ESI             ; identity
$+2    |.  8BC7         |MOV EAX,EDI             ; pTable
$+4    |.  E8 80C9F8FF  |CALL RemoveTableElement ; Args in EAX, EDX
$+9    |>  8D4E 01      |LEA ECX,[ESI+1]         ; next identity
*/
static const descriptor_sequence table_RemoveTableElement {
    bytes{0x8B, 0xD6,
          0x8B, 0xC7},
    read_call_rel32{ref(table::proc_RemoveTableElement)},
    bytes{0x8D, 0x4E, 0x01}
};

/*
$ ==>  |.  68 00010000  PUSH 100                     ; /datum_count = 0x100
$+5    |.  68 ECEE6500  PUSH OFFSET 0065EEEC         ; |szName = ASCII "actor"
$+A    |.  BB 24070000  MOV EBX,724                  ; |datum_size = 0x724
$+F    |.  E8 AB191100  CALL CreateTableFromAlocator
*/
static const descriptor_sequence table_CreateTableFromAllocator {
    bytes{0x68, 0x00, 0x01, 0x00, 0x00,
          0x68, -1, -1, -1, -1,
          0xBB, 0x24, 0x07, 0x00, 0x00},
    detour{detour_target,
           (void*)table::tramp_CreateTableFromAllocator,
           ref(table::proc_CreateTableFromAllocator)}
};

/*
$ ==>  |.  6A 20        PUSH 20                  ; /datum_count = 0x20
$+2    |.  68 E49A6600  PUSH OFFSET 00669AE4     ; |szName = ASCII "terminal output"
$+7    |.  BB 24010000  MOV EBX,124              ; |datum_size = 0x124
$+C    |.  E8 8D9F0300  CALL CreateTableFromHeap
*/
static const descriptor_sequence table_CreateTableFromHeap {
    bytes{0x6A, 0x20,
          0x68, -1, -1, -1, -1,
          0xBB, 0x24, 0x01, 0x00, 0x00},
    detour{detour_target,
           (void*)table::tramp_CreateTableFromHeap,
           ref(table::proc_CreateTableFromHeap)}
};

#define MAKE_PATCH(name, ...) detours::batch_descriptor{#name, name, __VA_ARGS__}
static const std::tuple patch_descriptors
{
    MAKE_PATCH(chat_DecodeChatUpdate),
    MAKE_PATCH(chat_SendChatToServer),
    MAKE_PATCH(chat_EnqueueChatEntry),

    MAKE_PATCH(console_TerminalPrintf),
    MAKE_PATCH(console_TerminalUpdate),
    MAKE_PATCH(console_Terminal),

    MAKE_PATCH(controls_GetUserActions),
    MAKE_PATCH(controls_ProcessUserControls),
    MAKE_PATCH(controls_ControlsState),

    MAKE_PATCH(engine_UpdateMapEntities),
    MAKE_PATCH(engine_ExtrapolateLocalUnitDelta),
    MAKE_PATCH(engine_UpdateBipedPosition),

    MAKE_PATCH(globals_GameTimeGlobals),
    MAKE_PATCH(globals_LocalPlayerGlobals),
    MAKE_PATCH(globals_TagsArrayHeader),
    MAKE_PATCH(globals_AllocatorGlobals),
    MAKE_PATCH(globals_CameraGlobals),
    MAKE_PATCH(globals_ChatGlobals),
    MAKE_PATCH(globals_MachineGlobals),
    MAKE_PATCH(globals_MapGlobals),
    MAKE_PATCH(globals_RuntimeSoundGlobals),
    MAKE_PATCH(globals_CommandLineArgs),
    MAKE_PATCH(globals_EditionString),

    MAKE_PATCH(init_ProcessStartup),
    MAKE_PATCH(init_LoadMapCacheSP),
    MAKE_PATCH(init_LoadMapCacheMP),
    MAKE_PATCH(init_InstantiateMap, ref(init::patch_InstantiateMap)),

    MAKE_PATCH(memory_GlobalFreeImport),

    MAKE_PATCH(object_GetObjectMarkers),
    MAKE_PATCH(object_GetUnitCameraPosition),
    MAKE_PATCH(object_ObjectPrototypes),

    MAKE_PATCH(raycast_CastRay),

    MAKE_PATCH(script_ScriptFunctionsArray),
    MAKE_PATCH(script_SymbolLookupProcedures),
    MAKE_PATCH(script_UserEvaluationBuffer),
    MAKE_PATCH(script_ProcessExpression),
    MAKE_PATCH(script_FunctionContextReturn),
    MAKE_PATCH(script_ParseScriptNodeExpected),
    MAKE_PATCH(script_PushEvalFrame),

    MAKE_PATCH(sound_DirectSoundInterfaces),
    MAKE_PATCH(sound_SecondarySoundBuffers),

    MAKE_PATCH(table_RemoveTableElement),
    MAKE_PATCH(table_CreateTableFromAllocator,
               ref(table::patch_CreateTableFromAllocator)),
    MAKE_PATCH(table_CreateTableFromHeap, ref(table::patch_CreateTableFromHeap))
};
#undef MAKE_PATCH

#define MAKE_MODULE(name) std::tuple(#name, name::Init, name::Debug)
constexpr std::array module_descriptors {
    MAKE_MODULE(globals), // top priority

    MAKE_MODULE(chat),
    MAKE_MODULE(console),
    MAKE_MODULE(controls),
    MAKE_MODULE(engine),
    MAKE_MODULE(init),
    MAKE_MODULE(memory),
    MAKE_MODULE(object),
    MAKE_MODULE(raycast),
    MAKE_MODULE(script),
    MAKE_MODULE(sound),
    MAKE_MODULE(table),
};

} // namespace anonymous
#undef MAKE_MODULE

namespace reve {

bool Init()
{
    auto apply_patches = [] (const auto&... descriptors)
    { return detours::batch_patches(descriptors...); };

    if (auto name = std::apply(apply_patches, descriptors::patch_descriptors)) {
        std::cout << "Failed to make patch " << *name << "\n";
        return false;
    }

    std::cout << "All patch/scan descriptors successful\n";
    for (const auto& [name, init, debug] : descriptors::module_descriptors) {
        std::cout << "Initializing module " << name << "\n";
        bool initialized = false;

        try {
            initialized = init();
        } catch (...) {
            std::cout << "Exception when initializing module " << name << "\n";
            initialized = false;
        }

        if (!initialized) {
            std::cout << "Failed to initialize module " << name << "\n";
            debug();
            return false;
        }

        std::cout << "Successfully initialized module " << name << "\n";
    }

    std::cout << "All module initialized successfully\n";
    Debug();
    return true;
}

void Debug()
{
    for (const auto& [name, init, debug] : descriptors::module_descriptors) {
        std::cout << "------------------------------\n"
                  << "Debug: " << name << "\n";
        debug();
    }
}

} // namespace reve::descriptors
