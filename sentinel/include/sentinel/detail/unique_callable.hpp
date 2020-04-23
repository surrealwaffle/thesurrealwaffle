
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef> // std::nullptr_t

#include <functional> // std::invoke_result
#include <utility>    // std::forward

namespace sentinel { namespace detail {

/** \brief A smart pointer that owns a function object.
 *
 * This type is not default nor copy-constructible, nor assignable.
 */
template<class F>
class unique_callable {
public:
    unique_callable()                       = delete; ///< DELETED
    unique_callable(std::nullptr_t)         = delete; ///< DELETED
    unique_callable(const unique_callable&) = delete; ///< DELETED

    unique_callable& operator=(const unique_callable&) = delete; ///< DELETED
    unique_callable& operator=(unique_callable&&)      = delete; ///< DELETED

    /** \brief Constructs the smart pointer to own \a p.
     *
     * If \a p is not heap-allocated, the behaviour of this object is undefined.
     */
    explicit unique_callable(F* p) noexcept : ptr(p) { }

    /** \brief Constructs the smart pointer by taking ownership from another.
     */
    unique_callable(unique_callable&& other) noexcept
        : ptr(other.ptr) { other.ptr = nullptr; };

    /** \brief Destroys the owned object.
     */
    ~unique_callable() { delete ptr; }

    /** \brief Invokes the callable object with the arguments \a args, as if by
     *         `std::invoke(f, std::forward<Args>(args)...)`, where `f`
     *         is the wrapped and pointed-to function object.
     *
     * Invoking an empty `unique_callable` results in undefined behaviour.
     *
     * \return The result of the invocation.
     */
    template<class... Args>
    std::invoke_result_t<F&, Args&&...> operator()(Args&&... args)
    { return std::invoke(*ptr, std::forward<Args>(args)...); }

private:
    F* ptr; ///< The pointer to the callable object.
};

} } // namespace sentinel::detail
