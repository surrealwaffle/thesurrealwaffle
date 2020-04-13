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

#ifndef NOGDI
#define NOGDI
#endif // NOGDI

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
