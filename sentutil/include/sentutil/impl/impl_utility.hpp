
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <utility> // std::forward

namespace sentutil { namespace utility { namespace impl {

template</*CRTP*/ class Derived>
struct dereference_container_interface {
public:
    template<class U = const Derived>
    auto begin() const -> decltype(static_cast<U*>(this)->dereference().begin())
    { return static_cast<U*>(this)->dereference().begin(); }

    template<class U = const Derived>
    auto end() const -> decltype(static_cast<U*>(this)->dereference().end())
    { return static_cast<U*>(this)->dereference().end(); }

    template<class U = const Derived>
    auto cbegin() const -> decltype(static_cast<U*>(this)->dereference().cbegin())
    { return static_cast<U*>(this)->dereference().cbegin(); }

    template<class U = const Derived>
    auto cend() const -> decltype(static_cast<U*>(this)->dereference().cend())
    { return static_cast<U*>(this)->dereference().cend(); }
};

template</*CRTP*/ class Derived>
struct dereference_function_interface {
    template<class U = const Derived, class... Args>
    auto operator()(Args&&... args) const
        -> decltype(static_cast<U*>(this)->dereference()(std::forward<Args>(args)...))
    { return static_cast<U*>(this)->dereference()(std::forward<Args>(args)...); }
};

template</*CRTP*/ class Derived>
struct dereference_subscript_interface {
    template<class Index, class U = const Derived>
    auto operator[](Index&& index) const
        -> decltype(static_cast<U*>(this)->dereference()[std::forward<Index>(index)])
    { return static_cast<U*>(this)->dereference()[std::forward<Index>(index)]; }
};

} } } // namespace sentutil::utility::impl
