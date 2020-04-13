#include <sentutil/console.hpp>

#include <cstdarg>
#include <cstdio>

#include <iterator>

namespace {

void vcprintf(sentinel::argbf const *color, char const* fmt, va_list args);

}

namespace sentutil { namespace console {

void cprintf(const sentinel::argbf& color, char const* fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    vcprintf(&color, fmt, args);
    va_end(args);
}


void cprintf(char const* fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    vcprintf(nullptr, fmt, args);
    va_end(args);
}

void process_expression(const char* expression)
{ sentinel_Console_ProcessExpression(expression); }

} } // namespace sentutil::console

namespace {

void vcprintf(sentinel::argbf const *color, char const* fmt, va_list args) {
    char buf[0xFF];
    buf[0xFE] = '\0';

    vsnprintf(buf, std::size(buf) - 1, fmt, args);
    sentinel_Console_TerminalPrint(color, buf);
}

}
