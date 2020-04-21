
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "chat.hpp"

#include <sentinel/config.hpp>

#include <utility> // std::move

namespace {

sentinel::resource_list<reve::chat::chat_update_filter> chat_update_filters;

} // namespace (anonymous)

namespace reve { namespace chat {

decode_chat_update_tproc  proc_DecodeChatUpdate = nullptr;
send_chat_to_server_tproc proc_SendChatToServer = nullptr;
enqueue_chat_entry_tproc  proc_EnqueueChatEntry = nullptr;

bool8 hook_DecodeChatUpdate(pointer             data     /*EAX*/,
                            regint                       /*EDX*/,
                            netmsg_chat_update* pChatMsg /*ECX*/)
{
    bool success = proc_DecodeChatUpdate(data, 0, pChatMsg);

    const auto [channel, player_index, text] = *pChatMsg;
    if (success)
        for (auto&& filter : chat_update_filters)
            success &= filter(channel, player_index, text);

    return success;
}

sentinel_handle InstallChatFilter(chat_update_filter&& filter)
{
    return ::chat_update_filters.push_back(std::move(filter));
}

bool Init()
{
    return proc_DecodeChatUpdate
        && proc_SendChatToServer
        && proc_EnqueueChatEntry;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", proc_DecodeChatUpdate);
    SENTINEL_DEBUG_VAR("%p", proc_SendChatToServer);
    SENTINEL_DEBUG_VAR("%p", proc_EnqueueChatEntry);
}

} } // namespace reve::chat
