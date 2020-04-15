#pragma once

#include <cstddef>

#include <sentinel/types.hpp>

#include <memory>

namespace sentutil {

/** \brief Acts as a restore point for the map instance data, which includes much
 *         (but not all) of the game state, and facilitates partial tick updates.
 *
 * A restore point is made as part of the `simulation` constructor, which makes a
 * copy of the map instance memory.
 *
 * The map objects can be advanced by calling \ref simulation::advance.
 * The map instance can be restored manually by calling \ref simulation::restore.
 *
 * These operations can be expensive to perform.
 */
class simulation {
    std::size_t             size_;          ///< Size (in bytes) of the restore data.
    std::unique_ptr<char[]> data_; ///< The restore point data.

public:
    /** \brief Creates a restore point of the map instance. */
    simulation();

    /** \brief Creates a restore point of the map instance and advances objects
     *         by the specified number of \a ticks.
     */
    simulation(long ticks) : simulation() { advance(ticks); }

    /** \brief Moves the restore point data and clears this restore point.
     */
    simulation(simulation&&) = default;

    /** \brief Moves the restore point data and clears this restore point.
     */
    simulation& operator=(simulation&&) = default;

    /** \brief Restores the map instance, if the restore point is not already cleared.
     */
    ~simulation() { restore(); }

    simulation(simulation const&)            = delete; ///< DELETED
    simulation& operator=(simulation const&) = delete; ///< DELETED

    /** \brief Advances the map objects by a number of \a ticks.
     *
     * This function acts directly on the game state and should only be called when
     * there is a restore point to restore to.
     */
    static void advance(long ticks);

    const char* data() const { return data_.get(); }

    std::size_t size() const { return size_; }

    const char* begin() const { return data(); }
    const char* end() const { return data() + size(); }

    /** \brief Reverts the map instance to that held by this restore point.
     *         Calling this function does not clear the restore point.
     */
    void restore();
};

} // namespace sentutil
