#include <sentutil/chat.hpp>

#include <cstdarg> // std::va_list
#include <cstring> // std::strlen
#include <cwchar>  // std::vswprintf

#include <iterator> // std::size
#include <string>   // std::wstring

namespace {

void send_chat_valist(int channel, const char* fmt, std::va_list args);

void send_chat_wvalist(int channel, const wchar_t* fmt, std::va_list args);

} // namespace (anonymous)

namespace sentutil { namespace chat {

void send_chat(utility::format_marker_type,
               int channel,
               const char* fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    send_chat_valist(channel, fmt, args);
    va_end(args);
}

void send_chat(utility::format_marker_type,
               const char* fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    send_chat_valist(0, fmt, args);
    va_end(args);
}

} } // namespace sentutil::chat

namespace {

void send_chat_valist(int channel, const char* fmt, va_list args)
{
    std::wstring wfmt(fmt, fmt + std::strlen(fmt));
    return send_chat_wvalist(channel, wfmt.c_str(), args);
}

void send_chat_wvalist(int channel, const wchar_t* fmt, va_list args)
{
    wchar_t buf[0xFF];
    buf[0xFE] = L'\0';

    std::vswprintf(buf, std::size(buf) - 1, fmt, args);
    sentinel_Chat_SendChatToServer(channel, buf);
}

} // namespace (anonymous)
