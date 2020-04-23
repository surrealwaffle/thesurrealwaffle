
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits> // std::is_same

#include "types.hpp"

#include <sentinel/base.hpp>

namespace reve { namespace chat {

using chat_update_filter = sentinel::function<bool(sentinel::enum_long  channel,
                                                   sentinel::index_byte player,
                                                   sentinel::h_cwcstr   text)>;

/** \brief A message sent over the network that indicates incoming player chat.
 */
struct netmsg_chat_update {
    enum_long   channel;      ///< 0-2: all,team,vehicle; others have no formatting
    index_ubyte player_index; ///< index into the players table, or -1 if no player
    h_wcstr     text;         ///< the message and just the message
}; static_assert(sizeof(netmsg_chat_update) == 0x0C);

/** \brief Decodes \a delta into \a pChatData.
 *
 * \param[in] data The encoded message data from the network.
 * \param[in] pChatMsg Output for the decoded message.
 *
 * \return `1` on decode success, otherwise `0`.
 */
using decode_chat_update_tproc __attribute__((cdecl, regparm(3)))
    = bool8(*)(P_IN    pointer             data     /*EAX*/,
               P_DUMMY regint                       /*EDX*/,
               P_OUT   netmsg_chat_update* pChatMsg /*ECX*/);

/** \brief Sends \a text over \a channel to the currently connected server.
 *
 * \a player_net_index must be the #network_index of the local player.
 *
 * \param[in] channel The channel to send \a text over.
 *                    `0` is all, `1` is team, `2` is vehicle.
 * \param[in] text The message to send.
 * \param[in] player_net_index The network index of the local player.
 */
using send_chat_to_server_tproc __attribute__((cdecl, regparm(2)))
    = void(*)(P_IN enum_long  channel          /*EAX*/,
              P_IN h_cwcstr   text             /*EDX*/,
              P_IN index_long player_net_index /*STACK*/);

/** \brief Updates the UI with another chat line consisting of \a text.
 *
 * \param[in] text The string to show in the chat box.
 *
 * \return The number of chat entries, including the one made during this call.
 */
using enqueue_chat_entry_tproc __attribute__((cdecl))
    = int32(*)(P_IN h_cwcstr text /*STACK*/);

extern decode_chat_update_tproc  proc_DecodeChatUpdate;
extern send_chat_to_server_tproc proc_SendChatToServer;
extern enqueue_chat_entry_tproc  proc_EnqueueChatEntry;

/** \brief A replacement procedure for \ref proc_DecodeChatUpdate.
 *
 * Decodes \a data and then runs \a pChatMsg through user filters to determine
 * whether or not to suppress the chat message.
 * This function stops on the first filter that desires to suppress the message.
 */
bool8 hook_DecodeChatUpdate(pointer             data     /*EAX*/,
                            regint                       /*EDX*/,
                            netmsg_chat_update* pChatMsg /*ECX*/)
                            __attribute__((cdecl, regparm(3)));

sentinel_handle InstallChatFilter(chat_update_filter&& filter);

bool Init();

void Debug();

static_assert(std::is_same_v<decode_chat_update_tproc,
                             decltype(&hook_DecodeChatUpdate)>);

} } // namespace reve::chat
