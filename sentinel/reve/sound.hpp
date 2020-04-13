#pragma once

#include <cstddef> // offsetof

#include <wunduws.hpp>
#include <mmsystem.h>
#include <dsound.h>

#include "types.hpp"

namespace reve { namespace sound {

struct direct_sound_interfaces {
    LPDIRECTSOUND8          lpDirectSound;   ///< The direct sound interface.
    LPDIRECTSOUNDBUFFER     lpPrimaryBuffer; ///< The direct sound primary buffer.
    LPDIRECTSOUND3DLISTENER lp3DListener;    ///< The direct sound 3d listener.
    bool8 unknown00;
    real  unknown01;
    bool8 unknown02;
    bool8 unknown03;
    int16 unknown04;
    int16 unknown05;
    int32 unknown06;
    int32 unknown07;
    int16 unknown08;
    bool8 unknown09;
    int32 unknown10;
    int32 unknown11;
}; static_assert(sizeof(direct_sound_interfaces) == 0x30);

struct secondary_sound_buffers {
    struct alignas(8) secondary_buffer_datum {
        char unknown0[0x670];
        LPDIRECTSOUNDBUFFER lpBuffer; ///< The associated secondary buffer.
        void* unknown1;
    }; static_assert(sizeof(secondary_buffer_datum) == 0x678);

    int16                  count; // 0x00 - number active
    secondary_buffer_datum buffers[51]; // 0x08
};

extern direct_sound_interfaces* ptr_DirectSoundInterfaces;
extern secondary_sound_buffers* ptr_SecondarySoundBuffers;

bool Init();

void Debug();

} } // namespace reve::sound
