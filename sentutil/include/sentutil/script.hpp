#pragma once

#include <type_traits> // std::enable_if
#include <utility>     // std::move

#include <sentinel/script.hpp>

#include <sentutil/impl/impl_script.hpp>
#include <sentutil/globals.hpp>
#include <sentutil/utility.hpp>

namespace sentutil { namespace script {

using impl::is_script_function;
using impl::is_script_function_v;

/** \brief Integrates a function into Halo's scripting engine.
 *
 * Due to the structure of Halo's function definitions, more specifically the flexible
 * array of parameter types, the name of the function must be provided as a
 * a compile-time string in order to uniquely identify the function.
 *
 * Registering a function under a name that collides with any script engine symbol,
 * including value types, built-in functions, globals, or installed functions,
 * results in undefined behaviour.
 *
 * The return type of and parameters to the function must satisfy certain conditions.
 * If these conditions are not met, then the program fails to compile.
 *
 * \todo Define and document behaviour around symbol collisions.
 * \todo Document function requirements
 * \todo Reduce memory footprint by de-templating the parser subroutine and
 *       moving a common snippet in the courotines into a separate function template.
 *
 * Sample usage:
 * \code{.cpp}
   if (install_script_function<"foo">(foo))
       std::cout << "function foo installed successfully\n";
   else
       std::cout << "function foo failed to install\n";
 * \endcode
 *
 * \tparam Name The exposed name of the function, as a compile-time string.
 * \param[in] f The function to expose.
 * \param[in] help_usage A description of what the function does.
 * \param[in] help_parameters A description of the parameters.
 *
 * \return `true` if the function was registered, otherwise `false`.
 */
template<utility::static_string Name, class R, class... Args>
std::enable_if_t<is_script_function_v<R(Args...)>, bool>
install_script_function(std::function<R(Args...)> f,
                        const char* help_usage      = nullptr,
                        const char* help_parameters = nullptr)
{
    using impl::argsize_type;
    using impl::registered_functions;
    using impl::create_evaluator_interface;
    using impl::get_default_parameters_help;

    if (!help_usage)
        help_usage = ""; // this is what Halo does

    if (!help_parameters) {
        static constexpr utility::static_string help = get_default_parameters_help<R, Args...>();
        help_parameters = +(help.string);
    }

    static_assert(sizeof...(Args) <= std::numeric_limits<argsize_type>::max(),
                  "parameter count must be representable by argsize_type");

    static const sentinel::script_function script_function {
        impl::script_value_traits<R>::index,
        Name,
        &impl::basic_script_node_parser<Args...>,
        &impl::basic_evaluator_coroutine<R, Args...>,
        help_usage,
        help_parameters,
        // access reqs: (PC) MP client only, (CE) MP host/client only
        // can reintroduce access into SP:
        //    PC: 0b01011001, CE: 0b00000001
        globals::game_edition == sentinel::GameEdition::combat_evolved
            ? (unsigned char)0b01001001 : (unsigned char)0b00010001,
        0, // param_count, set to 0 since we have our own parser at work
    };

    int function_index = 0;
    utility::handle handle = sentinel_script_InstallFunction(&script_function,
                                                             &function_index);
    if (handle) {
        registered_functions[function_index] = create_evaluator_interface(f);
        handle.make_managed();
        return true;
    } else {
        return false;
    }
}

template<utility::static_string Name, class R, class... Args>
std::enable_if_t<is_script_function_v<R(Args...)>, bool>
install_script_function(R (*f)(Args...),
                        const char* help_usage      = nullptr,
                        const char* help_parameters = nullptr)
{
    return install_script_function<Name>(std::function<R(Args...)>(f),
                                         help_usage,
                                         help_parameters);
}

} } // namespace sentutil::script
