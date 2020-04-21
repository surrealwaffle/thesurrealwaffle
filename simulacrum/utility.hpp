
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>

#include <functional>
#include <iterator>
#include <type_traits>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <sentinel/types.hpp>
#include <sentinel/structures/object.hpp>
#include <sentinel/tags/object.hpp>
#include <sentinel/tags/collision_model.hpp>

namespace simulacrum {

struct empty_struct { };

template<class ForwardIt>
struct rough_span {
    ForwardIt first;
    ForwardIt last;

    constexpr rough_span() = default;
    constexpr rough_span(ForwardIt first, ForwardIt last)
        : first(first)
        , last(last) { }

    constexpr ForwardIt begin() const noexcept { return first; }
    constexpr ForwardIt end()   const noexcept { return last;  }

    constexpr decltype(auto)
    operator[](typename std::iterator_traits<ForwardIt>::difference_type n)
    { return *std::next(first, n); }

    constexpr auto distance() const { return std::distance(first, last); }

    constexpr bool empty() const { return !(first != last); }
};

template<class Incrementable, class UnaryFunction>
rough_span<
    boost::transform_iterator<UnaryFunction, boost::counting_iterator<Incrementable>>
> constexpr make_rough_counting_span(Incrementable from,
                                     Incrementable to,
                                     UnaryFunction f)
{
    auto count_begin = boost::counting_iterator<Incrementable>(from);
    auto count_end   = boost::counting_iterator<Incrementable>(to);
    return {boost::transform_iterator(count_begin, f),
            boost::transform_iterator(count_end,   f)};
}

/** \brief Tests if a line segment intersects the sphere.
 *
 * \return `true` if the line segment intersects the sphere, otherwise `false`.
 */
bool intersects_segment_sphere(const sentinel::position3d&  segment_begin,
                               const sentinel::direction3d& segment_direction,
                               const sentinel::real&        segment_length,
                               const sentinel::position3d&  sphere_center,
                               const sentinel::real&        sphere_radius);

bool intersects_coll_pathfinding_spheres(const sentinel::position3d&  segment_begin,
                                         const sentinel::direction3d& segment_direction,
                                         const sentinel::real&        segment_length,
                                         const sentinel::object&      object,
                                         const sentinel::tags::collision_model& coll);

/** \brief Calls \a visitor on each scenery object on the current map, additionally
 *         supplying the object's tag.
 */
void visit_map_scenery(const std::function<void(sentinel::object&, sentinel::tags::object&)>& visitor);

} // namespace simulacrum
