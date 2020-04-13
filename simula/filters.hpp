#ifndef SIMULA__FILTERS_HPP
#define SIMULA__FILTERS_HPP

namespace sentinel {
    struct control_state;
    struct chat_receive_filter_info;
}

namespace simula::filters {

void OnMapLoad(char const* name);

void OnControlsUpdate(sentinel::control_state& controls, float dt, long ticks);

void OnChatReceive(sentinel::chat_receive_filter_info const& info);

}
#endif // SIMULA__FILTERS_HPP
