#pragma once

#include <cstddef> // std::size_t

#include <array>         // std::array
#include <functional>    // std::function
#include <memory>        // std::align
#include <optional>      // std::optional, std::nullopt
#include <unordered_map> // std::unordered_map
#include <string_view>   // std::basic_string_view
#include <type_traits>   // std::bool_constant, std::is_same,
#include <utility>       // std::index_sequence

#include <sentinel/types.hpp>
#include <sentinel/script.hpp>
#include <sentinel/globals.hpp>

#include <sentutil/utility.hpp>

// ------------------------------------------------
// Traits

namespace sentutil { namespace script { namespace impl {

template<class T>
struct script_value_traits {
    static constexpr bool defined   = false;
    static constexpr bool has_value = false;
};

template<>
struct script_value_traits<void> {
    static constexpr utility::static_string name = "void";

    static constexpr bool defined   = true;
    static constexpr bool has_value = false;
    static constexpr bool optional  = false;

    static constexpr sentinel::enum_short index = 4;


    static constexpr sentinel::script_value_union defaulted = {.u_long = 0l};
};

template<>
struct script_value_traits<bool> {
    static constexpr utility::static_string name = "bool";

    static constexpr bool defined   = true;
    static constexpr bool has_value = true;
    static constexpr bool optional  = false;

    static constexpr sentinel::enum_short index = 5;

    using type       = bool;
    using value_type = sentinel::boolean;

    static constexpr sentinel::script_value_union defaulted = {.u_boolean = false};

    static type extract(bool, sentinel::script_value_union* value)
    { return value->u_boolean; }

    static sentinel::script_value_union to_engine(const type& value)
    { return {.u_boolean = value}; }
};

template<>
struct script_value_traits<float> {
    static constexpr utility::static_string name = "real";

    static constexpr bool defined   = true;
    static constexpr bool has_value = true;
    static constexpr bool optional  = false;

    static constexpr sentinel::enum_short index = 6;

    using type       = float;
    using value_type = sentinel::real;

    static constexpr sentinel::script_value_union defaulted = {.u_real = 0.0f};

    static type extract(bool, sentinel::script_value_union* value)
    { return value->u_real; }

    static sentinel::script_value_union to_engine(const type& value)
    { return {.u_real = value}; }
};

template<>
struct script_value_traits<short> {
    static constexpr utility::static_string name = "short";

    static constexpr bool defined   = true;
    static constexpr bool has_value = true;
    static constexpr bool optional  = false;

    static constexpr sentinel::enum_short index = 7;

    using type       = short;
    using value_type = sentinel::h_short;

    static constexpr sentinel::script_value_union defaulted = {.u_short = 0};

    static type extract(bool, sentinel::script_value_union* value)
    { return value->u_short; }

    static sentinel::script_value_union to_engine(const type& value)
    { return {.u_short = value}; }
};

template<>
struct script_value_traits<long> {
    static constexpr utility::static_string name = "long";

    static constexpr bool defined   = true;
    static constexpr bool has_value = true;
    static constexpr bool optional  = false;

    static constexpr sentinel::enum_short index = 8;

    using type       = long;
    using value_type = sentinel::h_long;

    static constexpr sentinel::script_value_union defaulted = {.u_long = 0L};

    static type extract(bool, sentinel::script_value_union* value)
    { return value->u_long; }

    static sentinel::script_value_union to_engine(const type& value)
    { return {.u_long = value}; }
};

template<>
struct script_value_traits<const char*> {
    static constexpr utility::static_string name = "string";

    static constexpr bool defined   = true;
    static constexpr bool has_value = true;
    static constexpr bool optional  = false;

    static constexpr sentinel::enum_short index = 9;

    using type       = const char*;
    using value_type = sentinel::h_ccstr;

    static constexpr sentinel::script_value_union defaulted = {.u_string = ""};

    static type extract(bool, sentinel::script_value_union* value)
    { return value->u_string; }

    static sentinel::script_value_union to_engine(const type& value)
    { return {.u_string = value}; }
};

template<class Traits>
struct script_value_traits<std::basic_string_view<char, Traits>> {
    static constexpr utility::static_string name = "string";

    static constexpr bool defined   = true;
    static constexpr bool has_value = true;
    static constexpr bool optional  = false;

    static constexpr sentinel::enum_short index = 9;

    using type       = std::basic_string_view<char, Traits>;
    using value_type = sentinel::h_ccstr;

    static constexpr sentinel::script_value_union defaulted = {.u_string = ""};

    static type extract(bool, sentinel::script_value_union* value)
    { return value->u_string; }

    static sentinel::script_value_union to_engine(const type& value)
    { return {.u_string = value.data()}; }
};

template<class T>
struct script_value_traits<volatile T> : script_value_traits<T> { };

template<class T>
struct script_value_traits<const T> : script_value_traits<T> { };

template<class T>
struct script_value_traits<T&> : script_value_traits<T> { };

template<class T>
struct script_value_traits<std::optional<T>> : script_value_traits<T> {
    static_assert(!std::is_reference<T>::value);

    static constexpr bool optional = true;

    using type = std::optional<T>;

    static type extract(bool supplied, sentinel::script_value_union* value)
    {
        return supplied ? script_value_traits<T>::extract(supplied, value)
                        : type(std::nullopt);
    }
};

/** \brief Provides member constant `value` which is equal to `true` if function \a F
 *         can interface into the scripting engine, otherwise `value` is `false`.
 */
template<class F>
struct is_script_function : std::false_type { };

template<class R, class... Args>
struct is_script_function<R(Args...)>
    : std::bool_constant<
        script_value_traits<R>::defined && !script_value_traits<R>::optional &&
            (script_value_traits<Args>::has_value && ...) &&
            utility::are_bools_sorted({script_value_traits<Args>::optional...})
    > { };

template<class F>
inline constexpr bool is_script_function_v = is_script_function<F>::value;

} } } // namespace sentutil::script::impl

// ------------------------------------------------
// Utility Functions

namespace sentutil { namespace script { namespace impl {

using sentinel::identity;
using sentinel::script_node_type;
using sentinel::script_thread_type;

using argsize_type        = sentinel::h_ushort;
using evaluator_interface = std::function<void(identity<script_thread_type>  thread,
                                               argsize_type                  argc,
                                               sentinel::script_value_union* argv)>;

extern std::unordered_map<int, evaluator_interface> registered_functions;

inline sentinel::script_engine_functions_aggregate& engine_functions
    = sentinel_script_ScriptEngineFunctions;

template<class... Args>
sentinel::boolean
basic_script_node_parser(sentinel::index_short,
                         identity<script_node_type> script_node);

template<class R, class... Args>
void
basic_evaluator_coroutine(sentinel::index_short        function_index,
                          identity<script_thread_type> thread,
                          sentinel::boolean            initialize_frame);

template<class R, class... Args>
evaluator_interface
create_evaluator_interface(std::function<R(Args...)> f);

template<class R, class... Args>
constexpr auto get_default_parameters_help() /*-> static_string*/;

// ------------------------------------------------
// IMPLEMENTATION

template<class... Args>
sentinel::boolean
basic_script_node_parser(sentinel::index_short,
                         identity<script_node_type> node)
{
    struct argument_descriptor {
        sentinel::enum_short type_index;
        bool                 optional;
    };

    constexpr std::array<argument_descriptor, sizeof...(Args)> arg_descriptors {
        argument_descriptor{script_value_traits<Args>::index, script_value_traits<Args>::optional}...
    };

    node = node->value.u_script_node;
    for (auto& descriptor : arg_descriptors) {
        node = node->next_node;
        if (!node) // expression for this arg does not exist
            return descriptor.optional;
        else if (!engine_functions.parse_expected(node, descriptor.type_index))
            return false;
    }

    // ensure that no more than the required number of args were supplied
    return !node->next_node.valid();
}

template<class R, class... Args>
void
basic_evaluator_coroutine(sentinel::index_short        function_index,
                          identity<script_thread_type> thread,
                          sentinel::boolean            initialize_frame)
{
    using script_thread = sentinel::script_thread_type;
    using script_frame  = sentinel::script_frame_type;
    using sentinel::script_value_union;

    struct frame_locals_type {
        argsize_type arguments_evaluated;
        std::array<script_value_union, sizeof...(Args)> arguments;
    };

    script_thread* pThread = &*thread;
    script_frame*  pFrame  = pThread->frame;

    frame_locals_type* locals = [&] () -> frame_locals_type* {
        constexpr auto size      = sizeof(frame_locals_type);
        constexpr auto alignment = alignof(frame_locals_type);

        // recall: unary+ is the decay operator
        auto frame_begin      = +pFrame->data + pFrame->size;
        auto thread_stack_end = +pThread->data + sizeof(pThread->data);
        std::size_t space = static_cast<std::size_t>(thread_stack_end - frame_begin);
        void* ptr = frame_begin;

        if (!std::align(size, alignment, ptr, space))
            return nullptr;

        return initialize_frame
            ? new(ptr) frame_locals_type{0u, script_value_traits<Args>::defaulted...}
            : reinterpret_cast<frame_locals_type*>(ptr);
    }();

    // ensure locals could fit onto the stack
    if (!locals) {
        return engine_functions.return_from_context(script_value_traits<R>::defaulted,
                                                    thread);
    }

    // update frame size, this gets reset on each iteration in an update
    pFrame->size = ((char*)locals + sizeof(frame_locals_type)) - +pFrame->data;

    identity script_node = pFrame->script_node;
    script_node = script_node->value.u_script_node; // call expr
    script_node = script_node->next_node;

    // skip to the first unevaluated script node
    for (auto n = locals->arguments_evaluated; n; --n)
        script_node = script_node->next_node;

    // parser ensures script_node_id is invalid when all arguments are evaluated
    if (!script_node.valid()) {
        registered_functions[function_index](thread,
                                             locals->arguments_evaluated,
                                             locals->arguments.data());
        locals->~frame_locals_type(); // destroy locals
    } else {
        auto& argument = locals->arguments[locals->arguments_evaluated];
        ++locals->arguments_evaluated;
        engine_functions.push_evaluation_frame(script_node,
                                               thread,
                                               &argument);
    }
}

template<class R, class... Args>
evaluator_interface
create_evaluator_interface(std::function<R(Args...)> f)
{
    return [f] (identity<script_thread_type>  thread,
                argsize_type                  argc,
                sentinel::script_value_union* argv)
    {
        auto apply = [=]<std::size_t... I> (std::index_sequence<I...>)
        {
            if constexpr (std::is_void_v<R>) {
                f(script_value_traits<Args>::extract(I < argc, &argv[I])...);
                engine_functions.return_from_context(
                    script_value_traits<R>::defaulted,
                    thread);
            } else {
                auto return_value = f(
                    script_value_traits<Args>::extract(I < argc, &argv[I])...);
                engine_functions.return_from_context(
                    script_value_traits<R>::to_engine(return_value),
                    thread);
            }
        };

        return apply(std::make_index_sequence<sizeof...(Args)>{});
    };
}

template<class R, class... Args>
constexpr auto get_default_parameters_help()
{
    using utility::static_string;

    [[maybe_unused]]
    auto form_param_string = [] (const auto& traits) {
        if constexpr (traits.optional)
            return static_string("[<") + traits.name + static_string(">]");
        else
            return static_string("<") + traits.name + static_string(">");
    };

    return (static_string("") + ...
            + (form_param_string(script_value_traits<Args>{}) + static_string(" ")));
}

} } } // namespace sentutil::script::impl
