#ifndef SENTINEL__STRUCTURES__SOUND_HPP
#define SENTINEL__STRUCTURES__SOUND_HPP

#include <sentinel/types.hpp>

#include <sentinel/structures/table.hpp>

namespace sentinel {

struct runtime_sound_settings {
	fraction current_volume; ///< Controls the overall volume and interpolates to either `0` or `1`, regardless of #master_volume.
	fraction music_volume;   ///< The volume level of music.
	fraction master_volume;  ///< The overall volume level.
	fraction effects_volume; ///< The volume level of effects.
	int16 unknown0;          ///< 0x10, has something to do with the number of sounds/sound variety
	boolean  no_sound;       ///< Set to `true` to disable sound and `false` to enable sound.
	int8  unknown1;          ///< 0x13
	int16 variety;           ///< 0=low, 1=medium, 2=height, see #unknown0
	int16 PAD0;              ///< 0x16, unused
	int8  unknown2;          ///< 0x18
	table<void> *lpSounds;   ///< Pointer to the sounds table.
}; static_assert(sizeof(runtime_sound_settings) == 0x20);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(runtime_sound_settings, current_volume) == 0x00);
static_assert(offsetof(runtime_sound_settings, music_volume) == 0x04);
static_assert(offsetof(runtime_sound_settings, master_volume) == 0x08);
static_assert(offsetof(runtime_sound_settings, effects_volume) == 0x0C);
static_assert(offsetof(runtime_sound_settings, unknown0) == 0x10);
static_assert(offsetof(runtime_sound_settings, no_sound) == 0x12);
static_assert(offsetof(runtime_sound_settings, unknown1) == 0x13);
static_assert(offsetof(runtime_sound_settings, variety) == 0x14);
static_assert(offsetof(runtime_sound_settings, unknown2) == 0x18);
static_assert(offsetof(runtime_sound_settings, lpSounds) == 0x1C);
#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

}
#endif // SENTINEL__STRUCTURES__SOUND_HPP
