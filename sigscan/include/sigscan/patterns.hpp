#pragma once

#include <cstddef> // std:size_t, std::ptrdiff_t
#include <cstdint> // std::uintptr_t

#include <array>       // std::array
#include <functional>  // std::reference_wrapper
#include <tuple>       // std::tuple
#include <type_traits> // std::conditional, std::is_integral, std::is_pointer,
                       // std::remove_reference

#include "base.hpp"
#include "scan.hpp"

namespace sigscan { namespace patterns {

/** \brief A pattern that matches to a sequence of \a N bytes,
 *         where any negative value is used as a wildcard.
 */
template<std::size_t N>
struct bytes {

    auto operator()(byte*) const noexcept
    {
        return [it = data.cbegin(), end = data.cend()] (byte b) mutable {
            int const pattern = *it++;
            return pattern < 0 ?         (it != end ? scan_continue : scan_accept) :
                   !safe_integral_equal(pattern, b) ? scan_reject   :
                   it != end                        ? scan_continue : scan_accept;
        };
    }

    std::array<int, N> data;
};

/** \brief A pattern that matches to any sequence of \a N bytes.
 */
struct ignore {
    constexpr auto operator()(const byte*) const noexcept
    {
        return [n = N, N = this->N] (byte) mutable {
            return N == 0   ? scan_accept_noconsume :
                   --n != 0 ? scan_continue : scan_accept;
        };
    }

    std::size_t N;
};

/** \brief A pattern that emits an \a Integral value upon match completion.
 *
 * The captured integral value is emitted by `emit(value, first, last)`, where `value`
 * is the captured \a Integral value and `[first, last)` is the range producing the
 * match as \ref byte pointers.
 *
 * The value is emitted when the scanner would return ScanResult::scan_accept.
 *
 * The scanner captures #emit by value. If the user provides a reference-wrapped
 * emitter, then the scanner will effectively capture the emitter by reference.
 *
 * Scanners produced from this pattern can safely outlive the pattern.
 * See \ref make_emit_integral_scanner.
 */
template<class Integral, class Emitter>
struct emit_integral {
    static_assert(std::is_integral_v<Integral>);

    constexpr auto operator()(byte* first) const noexcept
    {
        constexpr auto size = sizeof(Integral);
        return [emit = this->emit, it = first, last = first + size] (byte) mutable {
            const auto scan_result = ++it != last ? scan_continue : scan_accept;
            if (scan_result == scan_accept) {
                const auto first = it - size;
                const auto value = bytes_to_integral<Integral>(first);
                emit(value, first, last);
            }
            return scan_result;
        };
    }

    Emitter emit;
};

/** \brief Creates the scanner that would result from an \ref emit_integral with
 *        \a emit as the emitter and \a first as start of scan.
 */
template<class Integral, class Emitter>
constexpr auto make_emit_integral_scanner(Emitter emit, byte* first)
{ return emit_integral<Integral, Emitter>{emit}(first); }

/** \brief A pattern whose scanner assigns to \a target a pointer to the byte
 *         on which the scanner was initialized. The scanner does not consume bytes.
 */
template<class Target>
struct capture_address {
    constexpr auto operator()(byte* first) const noexcept
    {
        return [first, target = this->target] (byte)
            { assign_target(target, first); return scan_accept_noconsume; };
    }

    Target target;
};

/** \brief A pattern that assigns to #target a captured \a Integral value.
 *
 * If \a Target is a reference type or reference-wrapped type, then scanners produced
 * by this pattern effectively capture #target by reference.
 * Otherwise, #target is captured by value.
 * In either case, scanners produced by this pattern may safely outlive the pattern.
 *
 * Assignment occurs by `unwrap(target) = value` where `target` is the captured
 * #target as described above and `value` is the matched \a Integral value.
 */
template<class Integral, class Target = std::reference_wrapper<Integral>>
struct capture_integral {
    static_assert(std::is_integral_v<Integral>);

    constexpr auto operator()(byte* first) const noexcept
    {
        auto emitter = [target = this->target]
                       (Integral value, auto&&...)
                       { assign_target(target, value); };
        return make_emit_integral_scanner<Integral>(emitter, first);
    }

    Target target;
};

/** \brief A pattern that assigns to #target an absolute \a Pointer interpreted from
 *         raw integer type \a Integral.
 */
template<
    class Integral,
    class Pointer,
    class Target = std::reference_wrapper<Pointer>
> struct capture_pointer {
    static_assert(std::is_pointer_v<Pointer>);

    constexpr auto operator()(byte* first) const noexcept
    {
        auto emitter = [target = this->target]
                       (Integral value, auto&&...) {
            assign_target(target, reinterpret_cast<Pointer>(value));
        };

        return make_emit_integral_scanner<Integral>(emitter, first);
    }

    Target target;
};

/** \brief A pattern that assigns to #target an address by relative \a Displacement.
 *
 * If \a Target is a reference type or reference-wrapped type, then scanners produced
 * by this pattern effectively capture #target by reference.
 * Otherwise, #target is captured by value.
 * In either case, scanners produced by this pattern may safely outlive the pattern.
 *
 * Assignment occurs by `unwrap(target) = reinterpret_cast<S>(first + Offset + value)`
 * where `first` is a pointer to the first byte of the match, `value` is the
 * matched \a Displacement value, and `S` is determined by the following rules:
 *  * if `Target` is a reference (or reference-wrapped) type to a non-member pointer,
 *    then `S` is the referenced pointer type;
 *  * otherwise `S` is `std::uintptr_t`.
 */
template<
    class Displacement,
    class Target
> struct capture_displacement {
    static_assert(std::is_integral_v<Displacement>);

    using interpret_type = std::conditional_t<
        std::is_pointer_v<std::remove_reference_t<unwrap_reference_t<Target>>>,
        std::remove_reference_t<unwrap_reference_t<Target>>,
        std::uintptr_t
    >;

    constexpr auto operator()(byte* first) const noexcept
    {
        auto emitter = [target = this->target,
                        offset = this->offset]
                       (Displacement value, byte* first, byte*) {
            assign_target(target,
                          reinterpret_cast<interpret_type>(first + offset + value));
        };
        return make_emit_integral_scanner<Displacement>(emitter, first);
    }

    Target         target;
    std::ptrdiff_t offset;
};

/** \brief Constructs a pattern that matches an integral value of type \a Displacement
 *         and assigns to \a target the resulting absolute address.
 */
template<class Displacement, class Target>
constexpr capture_displacement<Displacement, std::reference_wrapper<Target>>
make_capture_displacement(Target& target, std::ptrdiff_t offset = 0)
{ return {target, offset}; }

/** \brief Constructs a pattern that matches an integral value of type \a Displacement
 *         and assigns to \a target the resulting absolute address.
 */
template<class Displacement, class Target>
constexpr capture_displacement<Displacement, std::reference_wrapper<Target>>
make_capture_displacement(std::reference_wrapper<Target> target,
                          std::ptrdiff_t offset = 0)
{ return {target, offset}; }

template<class Displacement, class Target>
constexpr void make_capture_displacement(Target&&, std::ptrdiff_t = 0) = delete;

/** \brief Combines a sequence of \a Patterns into a single pattern.
 *
 * The scanner resulting from this pattern respects the rules regarding \a ScanResult.
 * See \ref continuation_scanner for details.
 */
template<class... Patterns>
class pattern_sequence
{
public:
    pattern_sequence() = default;
    pattern_sequence(const Patterns&... patterns) : patterns(patterns...) { }

    continuation_scanner operator()(byte* first) const
    { return make_continuation_scanner(patterns, first); }

private:
    std::tuple<Patterns...> patterns;
};

// ================
// Deduction Guides

template<class... T>
bytes(T...) -> bytes<sizeof...(T)>;

template<class Integral>
capture_integral(Integral&) -> capture_integral<Integral>;

} } // namespace sigscan::patterns

