
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentinel/console.hpp>

#include "reve/console.hpp"
#include "reve/script.hpp"

SENTINEL_API
void
sentinel_Console_TerminalPrint(const sentinel::argbf* color, sentinel::h_ccstr text)
{
    // ironically, this is how halo uses the proc_, even though it is variadic
    if (text)
        reve::console::proc_TerminalPrintf(color ? *color : nullptr, "%s", text);
}

SENTINEL_API
bool
sentinel_Console_ProcessExpression(sentinel::h_ccstr expression)
{
    return reve::script::proc_ProcessExpression(expression);
}
