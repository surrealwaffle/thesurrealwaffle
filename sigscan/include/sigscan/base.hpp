#pragma once

#include <climits> // CHAR_BIT
#include <cstddef> // std::size_t

#include <functional>  // std::reference_wrapper
#include <iterator>    // std::iterator_traits
#include <limits>      // std::numeric_limits
#include <optional>    // std::optional
#include <type_traits> // std::conditional, std::decay, std::enable_if,
                       // std::is_integral, std::is_lvalue_reference, std::is_same,
                       // std::is_signed, std::is_unsigned, std::is_void,
                       // std::make_unsigned, std::remove_reference
#include <utility>     // std::forward

namespace sigscan {

/** \brief The type used to represent the smallest unit of memory.
 */
using byte = unsigned char;

/** \brief If `T` is a `std::reference_wrapper<S>` for some `S`, then provides `type`
 *         as `S&`, otherwise defines `type` as `T`.
 */
template<class T>
struct unwrap_reference { using type = T; };

template<class T>
struct unwrap_reference<std::reference_wrapper<T>> { using type = T&; };

/** \brief Convenience alias for `typename unwrap_reference<T>::type`.
 */
template<class T>
using unwrap_reference_t = typename unwrap_reference<T>::type;

/** \brief An object whose destructor calls a \a Function.
 */
template<class Function>
class scope_guard {
public:
    scope_guard() = delete;
    scope_guard(const scope_guard&) = delete;

    scope_guard(scope_guard&&) = default;
    scope_guard(const Function& f) : f(f) { }
    scope_guard(Function&& f) : f(std::forward<Function>(f)) { }

    ~scope_guard() { (void)f(); }

private:
    Function f;
};

/** \brief Checks for equality between integrals \a s and \a t.
 *
 * If \a S and \a T are both signed or unsigned integral types, then comparison
 * is done acording to `s == t`.
 *
 * Otherwise, when \a S and \a T have different sign-age,
 * the unsigned value is converted to the signed type and then compared.
 * A compile-time assertion is performed to ensure this cast can be safely performed.
 *
 * \return `true` (`false`) if \a s and \a t are equal (not equal) according
 *         to the rules above.
 */
template<class S, class T>
constexpr bool
safe_integral_equal(const S& s, const T& t) noexcept
{
    if constexpr (std::is_signed_v<S> == std::is_signed_v<T>) {
        return s == t;
    } else if constexpr (std::is_signed_v<S> /* && std::is_unsigned_v<T> */) {
        static_assert(std::numeric_limits<S>::max() >= std::numeric_limits<T>::max(),
                      "supplied unsigned type cannot safely compare to signed type");
        return s == static_cast<S>(t);
    } else { // std::is_signed_v<T>
        return safe_integral_equal(t, s);
    }
}

/** \brief Reduces \a integral to the congruent value representable by the type
 *         `typename std::make_signed<Integral>::type`.
 */
template<class Integral>
constexpr auto safe_to_signed(Integral integral) noexcept
    -> std::enable_if_t<std::is_integral_v<Integral>, std::make_signed_t<Integral>>
{
    if constexpr (std::is_signed_v<Integral>) {
        return integral;
    } else { // std::is_unsigned_v<Integral>
        using SignedIntegral = std::make_signed_t<Integral>;

    // change __cplusplus > 201703L when C++20 comes out
    // C++20 unsigned->signed conversion is well-defined for unrepresentable
    // signed values, reducing modulo 2^N
    // this is also the behaviour of g++
#if defined(__GNUG__) || __cplusplus > 201703L
        return static_cast<SignedIntegral>(integral);
#else
        bool representable = integral <= std::numeric_limits<Integral>::max();
        return representable ? static_cast<SignedIntegral>(integral)
                             : -static_cast<SignedIntegral>(-integral);
#endif
    }
}

/** \brief Reverses the order of the bytes of \a integral, as determined by the
 *         boundaries formed by contiguous groups of CHAR_BIT bits.
 *
 * This function is portable but may not be the most efficient implementation.
 * GCC 7.1 through 9.2 with -O2, -Os, or -O3 emit a `bswap` on x86_64.
 * MSVC is not nearly as fortunate.
 */
template<class UnsignedIntegral>
constexpr auto bswap(UnsignedIntegral integral) noexcept
    -> typename std::enable_if<std::is_unsigned<UnsignedIntegral>::value,
                              UnsignedIntegral>::type
{
    UnsignedIntegral result = static_cast<UnsignedIntegral>(0);
    for (std::size_t n = 0; n != sizeof(integral); ++n) {
        UnsignedIntegral b = static_cast<unsigned char>(integral >> (CHAR_BIT * n));
        result |= b << (CHAR_BIT * (sizeof(integral) - 1 - n));
    }

    return result;
}

/** \brief Converts a sequence of `sizeof(Integral)` bytes (of type \ref byte)
 *         to an \a Integral value, respecting host byte-order.
 *
 * If \a Integral is a signed type, then conversion consists of reducing modulo `2^N`
 * (to be within range of \a Integral) the bytes as an unsigned \a Integral.
 *
 * The range `[it, it + N)` where `N` is `sizeof(Integral)` must be dereferenceable,
 * otherwise this function invokes undefined behaviour.
 */
template<class Integral, class InputIt>
constexpr
std::enable_if_t<
    std::is_integral_v<Integral>
        && std::is_same_v<typename std::iterator_traits<InputIt>::value_type, byte>,
    Integral
> bytes_to_integral(InputIt it) noexcept
{
    constexpr bool is_big_endian    = __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__;
    constexpr bool is_little_endian = __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
    static_assert(is_big_endian ^ is_little_endian, "unsupported byte-order");

    if constexpr (std::is_unsigned_v<Integral>) {
        Integral value = static_cast<Integral>(0);

        for (std::size_t n = 0; n != sizeof(Integral); ++n, ++it) {
            const auto b = static_cast<Integral>(static_cast<byte>(*it));

            if constexpr (is_big_endian)
                value = (value << CHAR_BIT) | b;
            else if constexpr (is_little_endian)
                value |= b << (CHAR_BIT * n);
        }

        return value;
    } else { // std::is_signed_v<Integral>()
        using UnsignedIntegral = std::make_unsigned_t<Integral>;
        return safe_to_signed(bytes_to_integral<UnsignedIntegral>(it));
    }
}

/** \brief Converts \a integral to a sequence of bytes output through \a out.
 *
 * If \a Integral is a signed type, then this function casts it to the unsigned
 * counterpart before converting the value to a sequence of bytes.
 */
template<class Integral, class OutputIt>
constexpr
std::enable_if_t<
    std::is_integral_v<Integral>
        && std::is_same_v<typename std::iterator_traits<OutputIt>::value_type, byte>,
    OutputIt
> integral_to_bytes(Integral integral, OutputIt out)
{
    constexpr bool is_big_endian    = __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__;
    constexpr bool is_little_endian = __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
    static_assert(is_big_endian ^ is_little_endian, "unsupported byte-order");

    if constexpr (std::is_unsigned<Integral>()) {
        if constexpr (is_big_endian)
            integral = bswap(integral);

        for (auto n = sizeof(integral); n; --n, ++out, integral >>= CHAR_BIT)
            *out = static_cast<byte>(integral);

        return out;
    } else { // std::is_signed_v<Integral>()
        using UnsignedIntegral = std::make_unsigned_t<Integral>;
        return integral_to_bytes(static_cast<UnsignedIntegral>(integral), out);
    }
}

template<class Target, class Value>
void assign_target(Target& target, Value&& value)
{
    target = std::forward<Value>(value);
}

template<class Target>
void assign_target(Target*& target, byte* value) {
    if constexpr (std::is_void_v<Target>)
        target = reinterpret_cast<Target*>(value);
    else
        target = reinterpret_cast<Target*>(__builtin_assume_aligned(value,
                                                                    alignof(Target)));
}

template<class Target, class Value>
void assign_target(std::optional<Target>& target, Value&& value)
{
    if (target)
        *target = std::forward<Value>(value);
}

template<class Target, class Value>
void assign_target(const std::reference_wrapper<Target>& target, Value&& value)
{
    assign_target(target.get(), std::forward<Value>(value));
}

template<class Target, class Value>
void assign_target(std::reference_wrapper<Target>& target, Value&& value)
{
    assign_target(target.get(), std::forward<Value>(value));
}

} // namespace sigscan
