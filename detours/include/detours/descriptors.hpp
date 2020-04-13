#pragma once

#include <functional>  // std::reference_wrapper
#include <optional>    // std::optional
#include <tuple>       // std::apply, std::tuple
#include <utility>     // std::move

#include <sigscan/sigscan.hpp>

#include "patch.hpp"

namespace detours { namespace descriptors {

// -----------------------------------------------------------------------------------

using sigscan::patterns::bytes;

// -----------------------------------------------------------------------------------

using sigscan::patterns::ignore;

// -----------------------------------------------------------------------------------

template<class Integral>
class read_integral {
public:
    using reference = std::reference_wrapper<Integral>;

    constexpr read_integral(reference target) : pattern{target} { }

    constexpr auto operator()(byte* first) const noexcept { return pattern(first); }

private:
    sigscan::patterns::capture_integral<Integral, reference> pattern;
};

// -----------------------------------------------------------------------------------

template<class Pointer>
class read_address {
public:
    using reference = std::reference_wrapper<Pointer>;

    constexpr read_address(const std::reference_wrapper<Pointer>& target)
        : pattern{target} { }

    constexpr auto operator()(byte* first) const noexcept { return pattern(first); }

private:
    sigscan::patterns::capture_address<reference> pattern;
};

// -----------------------------------------------------------------------------------

template<class Pointer>
class read_pointer {
public:
    using reference = std::reference_wrapper<Pointer>;

    constexpr read_pointer(reference target) : pattern{target} { }

    constexpr auto operator()(byte* first) const noexcept { return pattern(first); }

private:
    sigscan::patterns::capture_pointer<pointer_int, Pointer, reference> pattern;
};

// -----------------------------------------------------------------------------------

template<class Target>
class read_rel16 {
public:
    using reference = std::reference_wrapper<Target>;

    constexpr read_rel16(reference target, std::ptrdiff_t offset)
        : pattern{target, offset} { }

    constexpr auto operator()(byte* first) const noexcept { return pattern(first); }

private:
    sigscan::patterns::capture_displacement<displacement16, reference> pattern;
};

template<class Target>
class read_rel32 {
public:
    using reference = std::reference_wrapper<Target>;

    constexpr read_rel32(reference target, std::ptrdiff_t offset)
        : pattern{target, offset} { }

    constexpr auto operator()(byte* first) const noexcept { return pattern(first); }

private:
    sigscan::patterns::capture_displacement<displacement32, reference> pattern;
};

// -----------------------------------------------------------------------------------

template<class Function>
class read_call_rel32 {
public:
    using reference = std::reference_wrapper<Function>;

    constexpr read_call_rel32(reference function)
        : pattern{bytes{0xE8}, read_rel32{function, sizeof(displacement32)}} { }

    // scanner
    auto operator()(byte* first) const noexcept { return pattern(first); }
private:
    sigscan::patterns::pattern_sequence<bytes<1>, read_rel32<Function>> pattern;
};

// -----------------------------------------------------------------------------------

/** \brief Matches a `CALL <rel32>` instruction and redirects either the call
 *         or the function itself to a #new_target.
 */
template<class Function>
class detour {
public:
    constexpr detour(DetourType type, Function new_target)
        : type(type)
        , new_target(new_target)
        , original_function(std::nullopt) { }

    constexpr detour(DetourType type,
                     Function new_target,
                     std::reference_wrapper<Function> original_function)
        : type(type)
        , new_target(new_target)
        , original_function(original_function) { }

    constexpr detour(const detour& other)
        : type(other.type)
        , new_target(other.new_target)
        , original_function(other.original_function) { }

    operator=(const detour& other) = delete; // replace later
    operator=(detour&& other) = delete;      // replace later

    /** \brief Indicates how the redirection is to be implemented.
     */
    DetourType type;

    /** \brief Pointer to the function to redirect to.
     */
    Function new_target;

    /** \brief If a function reference is available, it is assigned a pointer to the
     *         original function detoured away from.
     */
    std::optional<std::reference_wrapper<Function>> original_function;

    auto operator()(byte* first) const noexcept
    { return pattern(first); }

    template<class OutputIt>
    bool action(OutputIt patch_out)
    {
        if (original_function)
            (*original_function).get() = function;

        byte* patch_site = nullptr;
        switch (type) {
        case detour_call:   patch_site = address; break;
        case detour_target: patch_site = reinterpret_cast<byte*>(function); break;
        }

        if (patch_site == nullptr)
            return false;

        patch p;
        sigscan::memory_range range{patch_site, patch_site + 5};
        if (auto hold = sigscan::hold_range_rwx(range))
            p = patch(patch_site, get_detour_data(type, patch_site, new_target));

        bool patched = static_cast<bool>(p);
        if (p)
            *patch_out++ = std::move(p);
        return patched;
    }

private:
    using pattern_type = sigscan::patterns::pattern_sequence<
        read_address<byte*>,
        read_call_rel32<Function>
    >;

    mutable byte*    address  = nullptr;
    mutable Function function = {/* ZERO INITIALIZED */};
    pattern_type pattern = {
        read_address{std::ref(address)},
        read_call_rel32{std::ref(function)}
    };
};

template<class Target, class Value>
struct perform_assignment_deref {
public:
    constexpr perform_assignment_deref(const Target& target, const Value& value)
        : target(target)
        , value(value) { }

    constexpr auto operator()(byte*) const noexcept
    { return [] (byte) { return sigscan::scan_accept_noconsume; }; }

    template<class OutputIt>
    bool action(OutputIt) {
        unwrap(target) = *(unwrap(value));
        return true;
    }

private:
    Target target;
    Value  value;
};

template<class Pointer, class Value>
class write_indirect {
public:
    constexpr write_indirect(const Pointer& pointer, const Value& value)
        : pointer(pointer)
        , value(value) { }

    constexpr auto operator()(byte*) const noexcept
    { return [] (byte) { return sigscan::scan_accept_noconsume; }; }

    template<class OutputIt>
    bool action(OutputIt patch_out)
    {
        if (auto p = patch(unwrap(pointer), unwrap(value))) {
            *patch_out++ = std::move(p);
            return true;
        }

        return false;
    }

private:
    Pointer pointer;
    Value   value;
};

template<class Pointer, class Value>
class assign_indirect {
public:
    constexpr assign_indirect(const Pointer& pointer, const Value& value)
        : pointer(pointer)
        , value(value) { }

    constexpr auto operator()(byte*) const noexcept
    { return [] (byte) { return sigscan::scan_accept_noconsume; }; }

    template<class OutputIt>
    bool action(OutputIt patch_out)
    {
        if (auto p = patch(assign_value, unwrap(pointer), unwrap(value))) {
            *patch_out++ = std::move(p);
            return true;
        }

        return false;
    }

private:
    Pointer pointer;
    Value   value;
};

/** \brief Combines a sequence of \a Descriptors into a single descriptor.
 */
template<class... Descriptors>
class descriptor_sequence {
public:
    descriptor_sequence() = default;
    constexpr descriptor_sequence(const Descriptors&... descriptors)
        : descriptors(descriptors...) { }

    sigscan::continuation_scanner operator()(byte* first) const
    { return sigscan::make_continuation_scanner(descriptors, first); }

    template<class OutputIt>
    bool action(OutputIt patch_out)
    {
        auto do_actions = [patch_out] (auto&... d)
            { return (perform_patch_action(d, patch_out) && ...); };
        return std::apply(do_actions, descriptors);
    }

    template<std::size_t I>
    const auto& get_descriptor() const { return std::get<I>(descriptors); }

private:
    std::tuple<Descriptors...> descriptors;
};

/** \brief Indicates to the \a make_patch overloads that the descriptor applies
 *         to all occurrences of the pattern, not just the first occurrence.
 */
template<class Descriptor>
class range_descriptor {
public:
    constexpr range_descriptor(const Descriptor& descriptor)
        : descriptor(descriptor) { }

    auto operator()(byte* first) const { return descriptor(first); }

    template<class OutputIt>
    auto action(OutputIt patch_out) { return descriptor.action(patch_out); }

private:
    Descriptor descriptor;
};

/** \brief A descriptor with the same scanner as \a WrappedDescriptor but whose
 *         action is overridden by an \a ActionFunction.
 *
 * The \a ActionFunction is invoked as `f(ptr, out)` where `ptr` is a `byte` pointer
 * to the match instance and `out` is the patch output iterator.
 */
template<class WrappedDescriptor, class ActionFunction>
class imbued_descriptor {
public:
    imbued_descriptor() = delete;
    imbued_descriptor& operator=(imbued_descriptor const&) = delete;
    imbued_descriptor& operator=(imbued_descriptor&&) = delete;

    imbued_descriptor(const WrappedDescriptor& wrapped_descriptor,
                      const ActionFunction& action_function)
        : descriptor(read_address{std::ref(site_address)},
                     wrapped_descriptor)
        , action_function(action_function) { }

    imbued_descriptor(const imbued_descriptor& other)
        : descriptor(read_address{std::ref(site_address)},
                     other.descriptor.template get_descriptor<1>())
        , action_function(other.action_function) { }

    auto operator()(byte* first) const { return descriptor(first); }

    template<class OutputIt>
    auto action(OutputIt out) { return action_function(site_address, out); }

private:
    descriptor_sequence<read_address<byte*>, WrappedDescriptor> descriptor;
    ActionFunction action_function;

    mutable byte* site_address;
};

// ================
// Deduction Guides

} } // namespace detours::descriptors
