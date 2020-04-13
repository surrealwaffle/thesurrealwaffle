#pragma once

#include <sentinel/fundamental_types.hpp>

// Types for decorating function parameters
/** \brief Indicates the parameter is used for input.
 */
#define P_IN

/** \brief Indicates the parameter is used for output.
 */
#define P_OUT

/** \brief Indicates the parameter is used for both input and output.
 */
#define P_INOUT

/** \brief Indicates the parameter is specified to satisfy constraints,
 *         such as obscure calling conventions.
 */
#define P_DUMMY

namespace reve {

using namespace sentinel::fundamental_types;

// -----------------------------------------------------------------------------------
// Types for low-level interfaces (semantic)

using regint  = std::int32_t; //< An integer with the same width as a register.
using pointer = void*;        //< Denotes a pointer to an unknown type.

template<class...>
using exotic_function = void*;

} // namespace reve
