#pragma once

#include <sentinel/config.hpp>

#include <cstddef> // std::nullptr_t, std::size_t

#include <functional>  // std::invoke, std::is_invocable_r
#include <iterator>    // std::advance, std::iterator_traits
#include <type_traits> // std::aligned_storage, std::enable_if
#include <utility>     // std::forward, std::move

#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>

#include <sentinel/types.hpp>

#include <sentinel/detail/inplace_ops_traits.hpp>
#include <sentinel/detail/unique_callable.hpp>

extern "C" {

/** \brief Provides user lifetime control over resources shared from the user library
 *         to the `sentinel` framework facilities.
 *
 * Handles are acquired by calling upon various `sentinel` functions for which the
 * life of some resource is tied to the lifetime of the user library.
 * When this resource is to expire, or the user library is to unload, the user must
 * call \ref sentinel_FreeResource with the handle.
 */
typedef struct sentinel_handle_type *sentinel_handle;

/** \brief Creates a handle for a resource that when freed invokes
 *         `release(handle, resource)`, where `handle` is the handle being returned.
 *
 * \param[in] resource The resource to create a handle for.
 * \param[in] release The callback invoked when the handle is being destroyed, or
 *                    `nullptr` for no action.
 *
 * \return A handle for \a resource or `nullptr` on failure.
 */
SENTINEL_API
sentinel_handle
sentinel_MakeHandle(void* resource,
                    void (*release)(sentinel_handle, void*));

/** \brief Frees the resource referred to by \a handle.
 *
 * If \a handle is `nullptr`, then this function simply returns.
 * Otherwise, if \a handle was created from `sentinel_MakeHandle(resource, release)`,
 * then this function effectively invokes `release(handle, resource)`.
 *
 * \param[in] handle A handle to the resource to release.
 */
SENTINEL_API
void
sentinel_FreeHandle(sentinel_handle handle);

/** \brief Retrieves the network index of the local player.
 *
 * \return The network index of the local player, or
 *         `-1` if the local player could not be found.
 */
SENTINEL_API
int
sentinel_GetLocalPlayerNetworkIndex();

} // extern "C"

namespace sentinel {

template<class>
class function; ///< UNDEFINED

/** \brief A replacement for `std::function` which is designed to be ABI-resistant,
 *         so as to be able to pass from client libraries to \a sentinel facilities.
 *
 * Unlike `std::function`, this type is neither assignable nor default-constructible.
 * Invoking an empty object of this type results in undefined behaviour.
 *
 * Sufficiently small target functions will be placed into storage of the wrapper,
 * avoiding further allocations to the heap and a layer of indirection.
 *
 * Consistency across compilers is only ABI-resistant under the following conditions:
 *  * bytes consist of the same number of bits;
 *  * pointers have the same layout.
 * Please note that this is not a portable solution if you choose lift this code.
 */
template<class R, class... Args>
class function<R(Args...)> {
    // DO NOT CHANGE THESE CONSTANTS
    static constexpr std::size_t storage_alignment = 4;
    static constexpr std::size_t storage_length    = 20;

    using call_proc    = R(*)(void*, Args&&...);
    using storage_type = std::aligned_storage_t<storage_length, storage_alignment>;
    using inplace_operations_traits = detail::basic_inplace_operations_traits;

    static_assert(storage_length >= sizeof(void*) &&
                  storage_alignment >= alignof(void*));
public:
    function()                = delete; ///< DELETED
    function(std::nullptr_t)  = delete; ///< DELETED
    function(const function&) = delete; ///< DELETED
    function& operator=(const function&) = delete; ///< DELETED

    /** \brief Constructs the function object from \a f as if by `std::move(f)`.
     *
     * This overload is selected when \a f can fit into internal storage.
     */
    template<
        class F,
        std::enable_if_t<
            sizeof(F) <= storage_length && alignof(F) <= storage_alignment &&
                std::is_invocable_r<R, F, Args...>::value,
            int> = 0
    > function(F f)
        : storage()
        , call(+[] (void* d, Args&&... args) -> R {
               return std::invoke(*std::launder(reinterpret_cast<F*>(d)),
                                  std::forward<Args>(args)...); })
        , operations(&detail::basic_inplace_ops<F>)
    { operations->move(&storage, &f); }

    /** \brief Constructs the target function from \a f as if by `std::move(f)`.
     *
     * This overload is selected when \a f cannot fit into internal storage.
     * In this case, the target function is constructed onto the heap.
     */
    template<
        class F,
        std::enable_if_t<
            !(sizeof(F) <= storage_length && alignof(F) <= storage_alignment) &&
                std::is_invocable_r<R, F, Args...>::value,
            int> = 0
    > function(F f)
        : function(detail::unique_callable(new F(std::move(f)))) { }

    /** \brief Constructs the target function by moving ownership from \a other.
     */
    function(function&& other)
        : storage()
        , call(other.call)
        , operations(other.operations)
    { operations->move(&storage, &other.storage); }

    /** \brief Destroys the owned target function and then move contructs the \a other
     *         target function into place.
     */
    function& operator=(function&& other)
    {
        operations->dtor(&storage);

        call = other.call;
        operations = other.operations;
        operations->move(&storage, &other.storage);

        return *this;
    }

    /** \brief Destroys the owned target function.
     */
    ~function() noexcept
    { operations->dtor(&storage); }

    /** \brief Invokes the target function, effectively by
     *         `std::invoke(f, std::forward<Args>(args)...)`.
     */
    R operator()(Args... args) const
    { return call(&storage, std::forward<Args>(args)...); }

private:
    mutable storage_type             storage; ///< Storage for the target function.
    call_proc                        call; ///< Function for invoking the target.
    const inplace_operations_traits* operations; ///< In-place destructor and move.
};

template<class BidirIt>
class strided_iterator
    : public boost::iterator_adaptor<strided_iterator<BidirIt>, BidirIt>
{
public:
    using stride_type = typename std::iterator_traits<BidirIt>::difference_type;

    strided_iterator() = default;

    explicit strided_iterator(BidirIt it, stride_type stride)
        : strided_iterator::iterator_adaptor(it)
        , stride(stride) { }

private:
    friend class boost::iterator_core_access;

    void increment() { this->base_reference() += stride; }
    void decrement() { this->base_reference() -= stride; }
    void advance(typename strided_iterator::iterator_adaptor::difference_type n)
        { this->base_reference += n * stride; }

    stride_type stride;
};

} // namespace sentinel

// Framework Internals

#ifdef SENTINEL_BUILD_DLL
#include <algorithm>   // std::find_if
#include <functional>  // std::function
#include <utility>     // std::move, std::pair

namespace sentinel {

/** \brief Creates a resource handle than invokes \a on_free when freed.
 *
 * This function is not present when building a client library.
 */
sentinel_handle callback_handle(const std::function<void(sentinel_handle)>& on_free);

/** \brief Returns a resource handle that calls `on_free()` when freed via
 *         \ref sentinel_FreeHandle.
 *
 * This function is not present when building a client library.
 */
sentinel_handle callback_handle(std::function<void(sentinel_handle)>&& on_free);

/** \brief A collection of values of type \a T associated with a \ref sentinel_handle
 *         that, when freed, removes the associated value from the vector.
 *
 * Value types are of `std::pair<sentinel_handle, T>`.
 * This collection does not provide for lookup by handle.
 *
 * This class template is not present when building a client library.
 */
template<class T>
class resource_list {
    using container_type = std::vector<std::pair<sentinel_handle, T>>;
    using container_iterator       = typename container_type::iterator;
    using container_const_iterator = typename container_type::const_iterator;

    static constexpr auto get_second = [] (auto& p) -> auto& { return p.second; };
public:
    auto begin() noexcept
    { return boost::transform_iterator(container.begin(), get_second); }

    auto end() noexcept
    { return boost::transform_iterator(container.end(), get_second); }

    auto begin() const noexcept
    { return boost::transform_iterator(container.cbegin(), get_second); }

    auto end() const noexcept
    { return boost::transform_iterator(container.cend(), get_second); }

    auto cbegin() const noexcept { return container.cbegin(); }
    auto cend() const noexcept { return container.cend(); }

    /** \brief Pushes a \a value onto the list by copy.
     *
     * \return A \ref sentinel_handle that removes the pushed value when freed, or
     *         `nullptr` if \a value could not be pushed or an exception occurred.
     */
    sentinel_handle push_back(const T& value) noexcept;

    /** \brief Pushes a \a value onto the list by move.
     *
     * \return A \ref sentinel_handle that removes the pushed value when freed, or
     *         `nullptr` if \a value could not be pushed or an exception occurred.
     */
    sentinel_handle push_back(T&& value) noexcept;

private:
    container_iterator erase(container_const_iterator pos)
    { return container.erase(pos); }

    container_const_iterator find_by_handle(sentinel_handle handle) const;

    container_type container;
};

template<class T>
typename resource_list<T>::container_const_iterator
inline resource_list<T>::find_by_handle(sentinel_handle handle) const
{
    return std::find_if(container.cbegin(), container.cend(),
                        [handle] (auto& p) { return handle == p.first; });
}

template<class T>
inline sentinel_handle resource_list<T>::push_back(const T& value) noexcept
{
    try {
        auto release = [this] (sentinel_handle handle)
            { this->erase(this->find_by_handle(handle)); };

        sentinel_handle handle = callback_handle(release);
        if (handle)
            container.emplace_back(handle, value);
        return handle;
    } catch (...) {
        return nullptr;
    }
}

template<class T>
inline sentinel_handle resource_list<T>::push_back(T&& value) noexcept
{
    try {
        auto release = [this] (sentinel_handle handle)
            { this->erase(this->find_by_handle(handle)); };

        sentinel_handle handle = callback_handle(release);
        if (handle)
            container.emplace_back(handle, std::move(value));
        return handle;
    } catch (...) {
        return nullptr;
    }
}

} // namespace sentinel

#endif // SENTINEL_BUILD_DLL
