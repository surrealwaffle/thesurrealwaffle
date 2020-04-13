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

void chat_filter(long channel, int index, std::wstring_view text);

} // namespace (anonymous)

namespace halo_debug {

bool load()
{
    using sentutil::script::install_script_function;
    return install_script_function<"sdbg_time_chat">(time_chat, "measures the time it takes for a string sent over a channel(default=all) to present in chat")
        && sentutil::chat::install_chat_filter(chat_filter);
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
        std::string_view t = text.value_or("test ping");
        time_chat_pong_string.clear();
        // transform is not necessarily correct but close enough
        std::transform(t.begin(), t.end(), std::back_inserter(time_chat_pong_string),
                       [] (char c) { return static_cast<wchar_t>(c); });
    }
    sentutil::chat::send_chat((int)channel.value_or(0L), time_chat_pong_string);
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

} // namespace (anonymous)
