
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SENTINEL__GAME_INIT
#define SENTINEL__GAME_INIT

/** \brief Function pointer type used in #MakeGameInitPatch to include custom setup. */
using CUSTOMINITPROC = void (*)(void);

/** \brief Hooks the function that reads and processes `init.txt` and adds a call to \a initProc.
 *
 * param[in] initProc The function to add to the call, or `nullptr` if no function should be called.
 * \return `true` if the patch was successful.
 */
bool MakeGameInitPatch(CUSTOMINITPROC initProc = nullptr);

#endif // SENTINEL__GAME_INIT
