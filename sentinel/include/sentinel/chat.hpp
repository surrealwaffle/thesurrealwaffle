
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>
#include <sentinel/base.hpp>

extern "C" {

/** \brief Sends a chat message to the server.
 *
 * \param[in] channel The channel to send \a text over.
 *                    Takes a value in the range `[0,2]` to indicate all, team, and
 *                    vehicle channels, respectively.
 * \param[in] \a text The chat text to send.
 */
SENTINEL_API
void
sentinel_Chat_SendChatToServer(sentinel::enum_long channel,
                               sentinel::h_cwcstr  text);

/** \brief Places a new entry into the chat box.
 *
 * This function is local to the client. It does not send \a text over the network.
 *
 * \param[in] text The text to display.
 */
SENTINEL_API
void
sentinel_Chat_EnqueueChatEntry(sentinel::h_cwcstr text);

/** \brief Installs a filter that receives incoming player chat from the server.
 *
 * The filter receives the channel, player index, and text, in that order, and
 * returns `true` if the message should be shown and `false` if the message should
 * be suppressed.
 *
 * If any chat filter returns `false`, then the message is not shown.
 * However, even if an earlier filter returns `false`, all filters will receive
 * the chat notification.
 *
 * \param[in] The chat filter to install.
 *
 * \return A handle to the installed filter, or
 *         `nullptr` if the filter could not be installed.
 */
SENTINEL_API
sentinel_handle
sentinel_Chat_InstallChatFilter(
    sentinel::function<bool(sentinel::enum_long  channel,
                            sentinel::index_byte player,
                            sentinel::h_cwcstr   text)>* filter);

} // extern "C""
