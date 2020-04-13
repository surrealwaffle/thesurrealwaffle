#include <sentinel/chat.hpp>
#include "reve/chat.hpp"

#include <utility> // std::move

#include <sentinel/base.hpp>

SENTINEL_API
void sentinel_Chat_SendChatToServer(sentinel::enum_long channel,
                                    sentinel::h_cwcstr  text)
{
    auto network_index = sentinel_GetLocalPlayerNetworkIndex();
    if (network_index != -1)
        reve::chat::proc_SendChatToServer(channel, text, network_index);
}

SENTINEL_API
void sentinel_Chat_EnqueueChatEntry(sentinel::h_cwcstr text)
{
    reve::chat::proc_EnqueueChatEntry(text);
}

// TODO: stub
SENTINEL_API
sentinel_handle
sentinel_Chat_InstallChatFilter([[maybe_unused]]
    sentinel::function<bool(sentinel::enum_long  channel,
                            sentinel::index_byte player,
                            sentinel::h_cwcstr   text)>* filter)
{
    return filter ? reve::chat::InstallChatFilter(std::move(*filter))
                  : nullptr;
}
