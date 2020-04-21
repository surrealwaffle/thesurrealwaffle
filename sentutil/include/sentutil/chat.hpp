
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits> // std::is_invocable_r
#include <utility>     // std::move

#include <sentinel/config.hpp>
#include <sentinel/base.hpp>
#include <sentinel/chat.hpp>

#include <sentutil/utility.hpp>

namespace sentutil { namespace chat {

/** \brief Submits a formatted chat message to the server over the supplied channel.
 *
 * \param[in] channel The channel to submit the message over.
 * \param[in] fmt     The format string, as in `std::wprintf`.
 */
void send_chat(utility::format_marker_type,
               int channel,
               const char* fmt, ...) SENTINEL_ATTRIBUTES(format(printf, 3, 4));

/** \brief Submits a formatted chat message to the server over the global channel.
 *
 * \param[in] fmt The format string, as in `std::wprintf`.
 */
void send_chat(utility::format_marker_type,
               const char* fmt, ...) SENTINEL_ATTRIBUTES(format(printf, 2, 3));

/** \brief Submits a chat message to the server over the specified channel.
 *
 * \param[in] channel The channel to submit the message over.
 * \param[in] text    The message to submit.
 */
template<class Traits>
void send_chat(int channel, const std::basic_string_view<char, Traits>& text)
{ return send_chat(format_marker, channel, "%.*s", (int)text.size(), text.data()); }

/** \brief Submits a chat message to the server over the specified channel.
 *
 * \param[in] channel The channel to submit the message over.
 * \param[in] text    The message to submit.
 */
inline void send_chat(int channel, const std::string_view& text)
{ return send_chat(format_marker, channel, "%.*s", (int)text.size(), text.data()); }

/** \brief Submits a chat message to the server over the specified channel.
 *
 * \param[in] channel The channel to submit the message over.
 * \param[in] text    The message to submit.
 */
template<class Traits>
void send_chat(int channel, const std::basic_string_view<wchar_t, Traits>& text)
{ return send_chat(format_marker, channel, "%.*ls", (int)text.size(), text.data()); }

/** \brief Submits a chat message to the server over the specified channel.
 *
 * \param[in] channel The channel to submit the message over.
 * \param[in] text    The message to submit.
 */
inline void send_chat(int channel, const std::wstring_view& text)
{ return send_chat(format_marker, channel, "%.*ls", (int)text.size(), text.data()); }

/** \brief Submits a chat message to the server over the global channel.
 *
 * \param[in] text The message to submit.
 */
template<class Traits>
void send_chat(const std::basic_string_view<char, Traits>& text)
{ send_chat(0, text); }

/** \brief Submits a chat message to the server over the global channel.
 *
 * \param[in] text The message to submit.
 */
inline void send_chat(const std::string_view& text)
{ send_chat(0, text); }

/** \brief Submits a chat message to the server over the global channel.
 *
 * \param[in] text The message to submit.
 */
template<class Traits>
void send_chat(const std::basic_string_view<wchar_t, Traits>& text)
{ send_chat(0, text); }

/** \brief Submits a chat message to the server over the global channel.
 *
 * \param[in] text The message to submit.
 */
inline void send_chat(const std::wstring_view& text)
{ send_chat(0, text); }

/** \brief Installs a filter for incoming chat.
 *
 * This overload is selected if \a f is invocable with
 *  * the channel index, of type `sentinel::enum_long`,
 *  * the player index, of type `sentinel::index_byte`, and
 *  * the text, of type `sentinel::h_ccstr` (`const char*`),
 * in that order, and the result of invocation returns a type that is implicitly
 * convertible to `bool`.
 *
 * All chat filters are updated, but the message is not shown if any return `false`.
 *
 * \param[in] f The filter to receives the channel, player index, and text and
 *              returns `false` to suppress the message.
 *
 * \return `true` if and only if the filter was successfully installed.
 */
template<class Filter>
std::enable_if_t<
    std::is_invocable_r_v<bool, Filter&, long, int, const wchar_t*>,
bool> install_chat_filter(Filter f)
{
    sentinel::function<bool(sentinel::enum_long,
                            sentinel::index_byte,
                            sentinel::h_cwcstr)> func(
        [f] (auto channel, auto index, auto text) { return f(channel, index, text); }
    );
    return utility::manage_handle(sentinel_Chat_InstallChatFilter(&func));
}

/** \brief Installs a filter for incoming chat.
 *
 * This overload is selected if \a f is invocable with
 *  * the channel index, of type `sentinel::enum_long`,
 *  * the player index, of type `sentinel::index_byte`, and
 *  * the text, of type `sentinel::h_ccstr` (`const char*`),
 * in that order, and the result of invocation returns `void`.
 *
 * All chat filters are updated, but the message is not shown if any return `false`.
 *
 * \param[in] f The filter to receives the channel, player index, and text.
 *
 * \return `true` if and only if the filter was successfully installed.
 */
template<class Filter>
std::enable_if_t<
    std::is_invocable_v<Filter&, long, int, const wchar_t*>
        && std::is_void_v<std::invoke_result_t<Filter&, long, int, const wchar_t*>>,
bool> install_chat_filter(Filter f)
{
    return install_chat_filter(
        [f] (auto&&... args) -> bool
            { return (f(std::forward<decltype(args)>(args)...), true); }
    );
}

} } // namespace sentutil::chat
