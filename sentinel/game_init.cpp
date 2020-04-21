
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "game_init.hpp"

#include <detours.hpp>

namespace {
    using namespace detours;

    using INITSETTINGSPROC = bool __attribute__((regparm(1), cdecl))(*)(char const*);
    INITSETTINGSPROC defaultInitSettingsFn = nullptr;
    CUSTOMINITPROC   customInitProc        = nullptr;

    bool __attribute__((regparm(1), cdecl)) MyInitSettings(char const* file)  {
        printf("original init settings fn at %p\n", (void*)defaultInitSettingsFn);

        bool result = defaultInitSettingsFn ? defaultInitSettingsFn(file) : true;

        printf("custom init:\n");
        if (customInitProc)
            customInitProc();
        printf("end custom init\n");

        return result;
    }
}

bool MakeGameInitPatch(CUSTOMINITPROC initProc) {
    constexpr patch_descriptor descriptor = {
        bytes{0x89, 0x4c, 0x24, 0x0c,
              0x89, 0x54, 0x24, 0x10,
              0x88, 0x44, 0x24, 0x14,
              0x8D, 0x44, 0x24, 0x0c},
        detour{DETOUR_TYPE::CALL, MyInitSettings, &defaultInitSettingsFn}
    };

    if (char const* name = BatchPatch(descriptor)) {
        printf("failed to initialize patch: %s\n", name);
        return false;
    }

    customInitProc = initProc;

    return true;
}
