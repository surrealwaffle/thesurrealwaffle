
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/types.hpp>
#include <sentinel/structures/table.hpp>

namespace sentinel {

/** Data pertaining to the input line in the console terminal. */
struct console_line_buffer {
    struct key_input {
        flags_byte flags; ///< bit 0: shift, bit 1: ctrl, bit 2: alt
        char character;   ///< ANSI, `-1` if out of ANSI range or if it is a control character
        int16 code;       ///< No idea, goes through a basic lookup table and is `-1` for unmapped keys
    };

    int16     available_input; ///< The number of inputs available in #input.
    key_input input[32];       ///< Buffers the key inputs to the console.

    argbf line_color;      ///< The color that #line_prefix and #line are displayed in.
    char  line_prefix[32]; ///< The text that prefixes the console line (`"halo) "`), null-terminated.
    char  line[256];       ///< The console input, null-terminated.

    // below is a sub-structure and is referenced on its own
    char* lpszLine;        ///< Points to #line.
    int16 max_length;      ///< The maximum length of #line.
    int16 cursor_position; ///< The 0-based index of the cursor in #line.
    int16 select_start;    ///< The 0-based index of the selection start, or
                           ///< `-1` if there is no selection.
                           ///< This can be greater or less than #cursor_position.
}; static_assert(sizeof(console_line_buffer) == 0x1C0);

/** Structure pertaining to the general usage of the console terminal. */
struct console_globals_type {
    boolean is_open;                 ///< `true` if the console is open, otherwise `false`.
    boolean is_enabled;              ///< `true` if the console is enabled, otherwise `false`.
    console_line_buffer line_buffer; ///< Pertains to input for the console terminal.
}; static_assert(sizeof(console_globals_type) == 0x1C4);

/** \brief Represents a line of text in the terminal.
 */
struct terminal_output_entry {
    identity_salt salt;           ///< The salt of the identity.
    identity<terminal_output_entry> next;     ///< The identity of the next newest entry in the linked list, or the invalid identity if this entry is the newest entry.
    identity<terminal_output_entry> previous; ///< The identity of the next oldest entry in the linked list, or the invalid identity if this entry is the oldest entry.
    boolean       contains_tab;   ///< `true` if and only if #text contains the string `"|t"`.
    char          text[0xFF];     ///< The text to display.
    int32         ZERO2;          ///< Just a zero? may be a safety null for #text
    argbf         color;          ///< The display color of #text.
    ticks_long    life_ticks;     ///< The remaining number of ticks to display this line for (assuming the console fix is active).
}; static_assert(sizeof(terminal_output_entry) == 0x124);

/** Structure that stores the doubly linked list of console terminal lines.
 * The following describes iteration order:
 *  * Oldest to newest: start with #oldest_entry, follow through \ref terminal_output_entry::next_entry.
 *  * Newest to oldest: start with #newest_entry, follow through \ref terminal_output_entry::previous_entry.
 */
struct terminal_type {
    using terminal_table_type = table_type<terminal_output_entry>;

    boolean              initialized;     ///< Set to `true` if the terminal is initialized, otherwise `false`.
    terminal_table_type* terminal_output; ///< The table that stores lines displayed in the console terminal.
    identity<terminal_output_entry> newest_entry; ///< The identity of the most recent terminal line, or the invalid identity for no entry.
    identity<terminal_output_entry> oldest_entry; ///< The identity of the oldest terminal line, or the invalid identity for no entry.
    console_line_buffer* line_buffer;     ///< A pointer to the console line data.

    boolean draw_cursor;        ///< Indicates that the console text cursor is shown.
    h_long  last_cursor_toggle; ///< Time in milliseconds when #draw_cursor toggled.

    boolean is_console_attached; ///< `true` if terminal output also goes to the
                                 ///< attached OS console, otherwise `false`.
    int32 unknown4; // 0x20
    int32 unknown5; // 0x24
    int32 unknown6; // 0x28
    int16 unknown7; // 0x2C
}; static_assert(sizeof(terminal_type) == 0x30);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(console_line_buffer, available_input) == 0x00);
static_assert(offsetof(console_line_buffer, input) == 0x02);
static_assert(offsetof(console_line_buffer, line_color) == 0x84);
static_assert(offsetof(console_line_buffer, line_prefix) == 0x94);
static_assert(offsetof(console_line_buffer, line) == 0xB4);
static_assert(offsetof(console_line_buffer, lpszLine) == 0x1B4);
static_assert(offsetof(console_line_buffer, max_length) == 0x1B8);
static_assert(offsetof(console_line_buffer, cursor_position) == 0x1BA);
static_assert(offsetof(console_line_buffer, select_start) == 0x1BC);

static_assert(offsetof(console_globals_type, is_open) == 0x00);
static_assert(offsetof(console_globals_type, is_enabled) == 0x01);
static_assert(offsetof(console_globals_type, line_buffer) == 0x04);

static_assert(offsetof(terminal_output_entry, next)== 0x004);
static_assert(offsetof(terminal_output_entry, previous)== 0x008);
static_assert(offsetof(terminal_output_entry, contains_tab)  == 0x00C);
static_assert(offsetof(terminal_output_entry, text)          == 0x00D);
static_assert(offsetof(terminal_output_entry, ZERO2)         == 0x10C);
static_assert(offsetof(terminal_output_entry, color)         == 0x110);
static_assert(offsetof(terminal_output_entry, life_ticks)    == 0x120);

static_assert(offsetof(terminal_type, initialized) == 0x00);
static_assert(offsetof(terminal_type, terminal_output) == 0x04);
static_assert(offsetof(terminal_type, newest_entry) == 0x08);
static_assert(offsetof(terminal_type, oldest_entry) == 0x0C);
static_assert(offsetof(terminal_type, line_buffer) == 0x10);
#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

} // namespace sentinel

