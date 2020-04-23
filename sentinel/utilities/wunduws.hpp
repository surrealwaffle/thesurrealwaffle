
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/** \file wunduws.hpp
 * \brief `#include`s \c windows.h with most functionality stripped.
 */

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#ifndef NOCRYPT
#define NOCRYPT
#endif // NOCRYPT

#ifndef NOSERVICE
#define NOSERVICE
#endif // NOSERVICE

#ifndef NMCX
#define NMCX
#endif // NMCX

#ifndef NOIME
#define NOIME
#endif // NOIME

#include <windows.h>
