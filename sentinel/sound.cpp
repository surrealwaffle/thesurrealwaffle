
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentinel/sound.hpp>
#include "reve/sound.hpp"

#include <algorithm> // std::clamp

SENTINEL_API
void
sentinel_Sound_SetPlaybackFrequencyModifier(float modifier)
{
    using reve::sound::ptr_DirectSoundInterfaces;
    using reve::sound::ptr_SecondarySoundBuffers;

    DSCAPS dsCaps = {/* ZERO INITIALIZED */};
    dsCaps.dwSize = sizeof(dsCaps);

    if (ptr_DirectSoundInterfaces->lpDirectSound == nullptr ||
        ptr_DirectSoundInterfaces->lpDirectSound->GetCaps(&dsCaps) != DS_OK)
        return;

    WAVEFORMATEX wfxFormat = {};
    auto get_format = [&wfxFormat] (auto lpBuffer)
        { return lpBuffer->GetFormat(&wfxFormat, sizeof(wfxFormat), NULL); };
    auto clamp_rate = [min = dsCaps.dwMinSecondarySampleRate,
                       max = dsCaps.dwMinSecondarySampleRate] (DWORD rate)
                       { return std::clamp(rate, min, max); };

    for (long i = 0, I = ptr_SecondarySoundBuffers->count; i < I; ++i) {
        LPDIRECTSOUNDBUFFER lpBuffer = ptr_SecondarySoundBuffers->buffers[i].lpBuffer;
        if (lpBuffer && get_format(lpBuffer) == DS_OK) {
            lpBuffer->SetFrequency(clamp_rate(modifier * wfxFormat.nSamplesPerSec));
        }
    }
}
