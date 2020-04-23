
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef> // std::size_t
#include <cstdint> // std::int16_t, std::int32_t, std::uint16_t, std::uint32_t,
                   // std::uintptr_t

#include <array>       // std::array
#include <type_traits> // std::enable_if, std::false_type, std::is_integral, std::is_pointer, std::remove_cv, std::true_type, std::void_t,
#include <utility>     // std::declval

#include <sigscan/base.hpp>

namespace detours { namespace descriptors {

template<class Descriptor>
class range_descriptor; // forward declaration

enum DetourType {
    /** \brief Indicates that the detour should be placed on the call instruction.
     *         Only the captured call site is redirected.
     */
    detour_call,

    /** \brief Indicates that the detour should be placed at the function start.
     *         A jump is placed at the start of the function body.
     */
    detour_target
};

} } // detours::descriptors

namespace detours {

using byte = sigscan::byte;

using pointer32 = std::uint32_t;

using pointer_int = pointer32;

using displacement16 = std::int16_t;
using displacement32 = std::int32_t;

using udisplacement16 = std::uint16_t;
using udisplacement32 = std::uint32_t;

/** \brief If `s.action()` is well-formed, where `s` is of type `T&&`, provides
 *         constant member `value` as `true`, otherwise provides `value` as `false`.
 */
template<class T, class OutputIt, class = std::void_t<>>
struct has_action : std::false_type { };

template<class T, class OutputIt>
struct has_action<
    T,
    OutputIt,
    std::void_t<decltype(std::declval<T>().action(std::declval<OutputIt>()))>
> : std::true_type { };

template<class>
struct is_range_descriptor : std::false_type { };

template<class Descriptor>
struct is_range_descriptor<
    descriptors::range_descriptor<Descriptor>
    > : std::true_type { };

/** \brief Performs the patch action associated with \a descriptor, if available.
 *         If \a descriptor has no associated action, then this function returns.
 *
 * See \ref has_action.
 *
 * \param[in,out] patch_out An output iterator to store patches from the patch action.
 *
 * \return `static_cast<bool>(descriptor.action())` if \a descriptor has an action, or
 *         `true` if there is no action associated with \a descriptor.
 */
template<class Descriptor, class OutputIt>
bool perform_patch_action(Descriptor& descriptor,
                          [[maybe_unused]] OutputIt patch_out)
{
    if constexpr (has_action<Descriptor, OutputIt&>::value)
        return static_cast<bool>(descriptor.action(patch_out));
    else
        return true;
}

/** \brief Returns a sequence of \a integrals as an array of bytes.
 */
template<class... Integrals>
constexpr auto as_byte_array(const Integrals&... integrals) noexcept
    -> std::enable_if_t<(std::is_integral_v<Integrals> && ...),
                        std::array<byte, (sizeof(Integrals) + ...)>>
{
    std::array<byte, (... + sizeof(Integrals))> result = {/* ZERO INITIALIZED */};

    auto serialize = [it = result.begin()] (const auto& integral) mutable
        { it = sigscan::integral_to_bytes(integral, it); };

    (void)(serialize(integrals), ...);

    return result;
}

template<class Pointer>
constexpr auto pointer_to_integral(Pointer pointer) noexcept
    -> std::enable_if_t<std::is_pointer_v<Pointer>, pointer_int>
{ return reinterpret_cast<pointer_int>(pointer); }

template<class Pointer>
constexpr auto calculate_displacement32(const byte* site,
                                        Pointer pointer,
                                        std::ptrdiff_t offset = 0) noexcept
    -> std::enable_if_t<std::is_pointer_v<Pointer>, displacement32>
{
    std::uintptr_t site_integral = reinterpret_cast<std::uintptr_t>(site + offset);
    std::uintptr_t target_integral = reinterpret_cast<std::uintptr_t>(pointer);
    auto displacement_amount = target_integral - site_integral;
    return sigscan::safe_to_signed(static_cast<udisplacement32>(displacement_amount));
}

/** \brief Returns an array of bytes that would write a detour of \a type to \a target
 *         at the supplied address \a site.
 */
template<std::size_t Nops = 0, class Pointer>
constexpr auto get_detour_data(descriptors::DetourType type,
                               const byte* site,
                               Pointer target)
{
    // CALL / JMP respectively
    const byte opcode = type == descriptors::detour_call ? byte(0xE8) : byte(0xE9);
    const auto displacement = calculate_displacement32(site, target, 5);
    auto call_bytes = as_byte_array(opcode, displacement);

    // zero-initialized
    std::array<byte, std::tuple_size_v<decltype(call_bytes)> + Nops> data = {};
    data.fill(0x90); // nop
    {
        auto dst = data.begin();
        for (auto& b : call_bytes)
            *dst++ = b;
    }

    return data;
}

template<class T>
constexpr T& unwrap(T& t) noexcept { return t; }

template<class T>
constexpr T& unwrap(const std::reference_wrapper<T>& t) { return t.get(); }

template<class T>
constexpr T& unwrap(std::reference_wrapper<T>& t) { return t.get(); }

} // namespace detours
