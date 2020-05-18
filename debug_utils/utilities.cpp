
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "utilities.hpp"

#include <algorithm>
#include <chrono>
#include <optional>
#include <ratio>
#include <string_view>

#include <sentutil/all.hpp>

namespace {

/** @brief Sends @a text over @a channel and displays the time it takes to receive it.
 *
 * Effectively measures application-to-application latency via chat, which may be
 * useful for some servers that disable ping display or for servers which do not
 * appropriately measure this latency such as those running SAPP with the ping
 * measurement enhancements.
 */
void time_chat(std::optional<std::string_view> text,
               std::optional<long>             channel);

bool tantrum_digital(std::string_view input, short period, long hits);

/** \brief Sets the player to crouch in a sequence resembling the car horn tune.
 */
void la_cucaracha();

/** @brief If enabled, the user will turn by 180 degrees every tick, but movement
 *         controls remain consistent with relative direction.
 *
 * This command is used to demonstrate that when processing input, turns take effect
 * before directional movement.
 *
 * @param[in] enabled `true` to enable the described behaviour, otherwise
 *                    `false` to disable.
 *                    If supplied an empty optional, no change takes place.
 * \return `true` if this setting is enabled, otherwise `false`.
 */
bool sdbg_flip_move(std::optional<bool> enabled_);

void chat_filter(long channel, int index, std::wstring_view text);

void controls_filter(sentinel::digital_controls_state& digital,
                     sentinel::analog_controls_state& analog,
                     float seconds,
                     long ticks);

float pending_yaw_turn = 0.0f;

} // namespace (anonymous)

namespace halo_debug {

bool load()
{
    using sentutil::script::install_script_function;
    return install_script_function<"sdbg_time_chat">(time_chat, "measures the time it takes for a string sent over a channel(default=all) to present in chat")
        && install_script_function<"sdbg_tantrum_digital">(tantrum_digital, "hits a key repeatedly periodically", "<string:action> <short:period> <long:count>")
        && install_script_function<"sdbg_la_cucaracha">(la_cucaracha)
        && install_script_function<"sdbg_turn_yaw">(+[] (float f) { pending_yaw_turn += f; })
        && install_script_function<"sdbg_flip_move">(sdbg_flip_move, "allows for movement in a consistent direction while turning the character 180 degrees every tick")
        && sentutil::chat::install_chat_filter(chat_filter)
        && sentutil::controls::install_controls_filter(controls_filter);
}

} // namespace halo_debug

namespace {

std::chrono::steady_clock::time_point time_chat_start_time;
std::wstring                          time_chat_pong_string;
void time_chat(std::optional<std::string_view> text,
               std::optional<long>             channel)
{

    time_chat_start_time = std::chrono::steady_clock::now();
    {
        std::string_view t = text.value_or("test");
        time_chat_pong_string.clear();
        // transform is not necessarily correct but close enough
        std::transform(t.begin(), t.end(), std::back_inserter(time_chat_pong_string),
                       [] (char c) { return static_cast<wchar_t>(c); });
    }
    sentutil::chat::send_chat((int)channel.value_or(0L), time_chat_pong_string);
}

sentinel::frames_ubyte sentinel::digital_controls_state::* tantrum_action = nullptr;
short tantrum_period = 0;
long  tantrum_hits = 0;
long  tantrum_ticks_to_hit = 0;
bool tantrum_digital(std::string_view input, short period, long hits)
{
#define TANTRUM_CASE(name) if (input == #name) tantrum_action = &sentinel::digital_controls_state::name
    TANTRUM_CASE(jump);
    else TANTRUM_CASE(switch_grenade);
    else TANTRUM_CASE(action);
    else TANTRUM_CASE(switch_weapons);
    else TANTRUM_CASE(melee);
    else TANTRUM_CASE(flashlight);
    else TANTRUM_CASE(secondary_trigger);
    else TANTRUM_CASE(primary_trigger);
    else TANTRUM_CASE(accept);
    else TANTRUM_CASE(cancel);
    else TANTRUM_CASE(crouch);
    else TANTRUM_CASE(zoom);
    else TANTRUM_CASE(show_scores);
    else TANTRUM_CASE(reload);
    else TANTRUM_CASE(pick_up_weapon);
    else TANTRUM_CASE(say);
    else TANTRUM_CASE(say_team);
    else TANTRUM_CASE(say_vehicle);
    else return (sentutil::console::cprintf({1, 1}, "no action named \"%s\"", input.data()), false);

    period = std::max((short)1, period);
    tantrum_period       = period;
    tantrum_hits         = hits;
    tantrum_ticks_to_hit = period;
    return true;
}

int cucaracha_index = -1;
int cucaracha_subindex = 0;
void la_cucaracha()
{
    cucaracha_index = 0;
    cucaracha_subindex = 0;
}

struct {
    bool enabled;
    bool is_flipped;
} flip_move_settings;

bool sdbg_flip_move(std::optional<bool> enabled_)
{
    if (enabled_) {
        const bool enabled = enabled_.value();
        const bool previously_enabled = flip_move_settings.enabled;

        flip_move_settings.enabled = enabled;
        if (enabled && !previously_enabled)
            flip_move_settings.is_flipped = false;
    }

    return flip_move_settings.enabled;
}

void chat_filter(long channel, int index, std::wstring_view text)
{
    const bool is_user = sentutil::globals::players[index].network_index == sentinel_GetLocalPlayerNetworkIndex();

    if (is_user && text == time_chat_pong_string) {
        time_chat_pong_string.clear();
        auto now = std::chrono::steady_clock::now();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - time_chat_start_time);
        sentutil::console::cprintf("time_chat result: %ld ms", (long)millis.count());
    }
}

void controls_filter([[maybe_unused]] sentinel::digital_controls_state& digital,
                     [[maybe_unused]] sentinel::analog_controls_state& analog,
                     [[maybe_unused]] float seconds,
                     [[maybe_unused]] long ticks)
{
    // pending_yaw_turn
    [&] {
        analog.turn_left += pending_yaw_turn;
        pending_yaw_turn = 0.0f;
    }();

    // sdbg_flip_move
    [&] {
        using sentutil::constants::pi;
        auto& [enabled, is_flipped] = flip_move_settings;

        // sdbg_flip_move must be temporarily enabled while is_flipped to return
        // the user to an unflipped state, otherwise there is a 50/50 chance of
        // leaving the user turned around
        if (!enabled && !is_flipped)
            return;

        if (ticks > 0L) {
            is_flipped = !is_flipped;
            analog.turn_left = is_flipped ? pi : -pi;
        }

        if (is_flipped) {
            analog.move_forward = -analog.move_forward;
            analog.move_left = -analog.move_left;
        }
    }();

    if (ticks && tantrum_action != nullptr && tantrum_hits != 0) {
        if ((tantrum_ticks_to_hit -= ticks) <= 0) {
            digital.*tantrum_action = 1;

            while ((tantrum_ticks_to_hit += tantrum_period) <= 0) { /* DO NOTHING */ }
            if (tantrum_hits > 0)
                --tantrum_hits;

            if (tantrum_hits == 0)
                sentutil::console::cprintf({1, 0, 1}, "tantrum done");
        } else {
            digital.*tantrum_action = 0;
        }
    }

    {
        struct cucaracha_point {
            int duration;
            int pause;
        };

        constexpr cucaracha_point cucaracha_data[] {
            {4, 2},
            {4, 2},
            {4, 2},
            {7, 2},
            {6, 3},
            {3, 2},
            {3, 2},
            {3, 2},
            {7, 2},
            {6, 4},
            {5, 1},
            {5, 1},
            {3, 1},
            {2, 1},
            {2, 1},
            {2, 1},
            {5, 0},
        };

        auto get_pause_end = [] (const cucaracha_point& p) { return p.duration + p.pause; };

        if (cucaracha_index >= 0 && (unsigned)cucaracha_index >= std::size(cucaracha_data)) {
            cucaracha_index    = -1;
            cucaracha_subindex = 0;
        }

        if (cucaracha_index >= 0) {
            while ((unsigned)cucaracha_index < std::size(cucaracha_data) && cucaracha_subindex >= get_pause_end(cucaracha_data[cucaracha_index])) {
                ++cucaracha_index;
                cucaracha_subindex = 0;
            }

            if ((unsigned)cucaracha_index < std::size(cucaracha_data)) {
                digital.crouch = cucaracha_subindex < cucaracha_data[cucaracha_index].duration;
                cucaracha_subindex += ticks;
            }
        }
    }
}

} // namespace (anonymous)
