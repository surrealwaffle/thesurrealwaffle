/** \file doxygen.hpp
 *
 * \brief Documentation for namespaces and modules.
 */

/** \brief Utilities brought by the `sentinel` library. */
namespace sentinel {

    /** \brief State and functions for installing hooks and fixes.
     *
     * Anything in this namespace is solely for the systems layer.
     */
    namespace patches { }

}

/** \defgroup numerics Numeric Types
 *
 * \brief Basic types and utility for interop with Halo primitive values.
 *
 * Use these types when extending the `sentinel` library.
 * Be consistent and sensible about the use of unsigned integers;
 * Halo favors signed integers even in contexts where it is not necessarily practical.
 */

/** \defgroup common_types Common Types
 *
 * \brief Composite types used throughout Halo.
 */

/** \defgroup exports Exports
 *
 * \brief Symbols exported through the DLL.
 */
