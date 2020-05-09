#include "bot_config.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>

#include <algorithm>
#include <string>
#include <tuple>
#include <type_traits>

#include <sentutil/all.hpp>

namespace {

bool install_config_field_accessors();

simulacrum::config::ConfigState config_state;

/** \brief Sets the name of the client to be used when connecting to a server.
 */
void set_name(const char* name);

/** \brief Modifies the name set by #set_name to be unique given the client index from
 *         the config_state.
 */
void obfuscate_name();

}

namespace simulacrum { namespace config {

WeaponConfig& ConfigState::get_weapon_config(const weapon_tag_id& id)
{
    if (!id)
        return default_weapon_config;
    auto it = weapon_configs.find(id.raw);
    return it != weapon_configs.end() ? it->second : default_weapon_config;
}

WeaponConfig& ConfigState::get_weapon_config(const sentinel::identity<sentinel::weapon>& weapon_id)
{
    return get_weapon_config(weapon_id ? weapon_tag_id{weapon_id->object.tag.raw} : sentinel::invalid_identity);
}

const WeaponConfig& ConfigState::get_weapon_config(const weapon_tag_id& id) const
{
    if (!id)
        return default_weapon_config;
    auto it = weapon_configs.find(id.raw);
    return it != weapon_configs.end() ? it->second : default_weapon_config;
}

const WeaponConfig& ConfigState::get_weapon_config(const sentinel::identity<sentinel::weapon>& weapon_id) const
{
    return get_weapon_config(weapon_id ? weapon_tag_id{weapon_id->object.tag.raw} : sentinel::invalid_identity);
}

void ConfigState::select_weapon(const weapon_tag_id& weapon)
{
    selected_weapon = weapon;
    weapon_configs.insert({weapon.raw, default_weapon_config});
}

void ConfigState::select_default_weapon()
{
    selected_weapon = sentinel::invalid_identity;
}

void ConfigState::deselect_weapon()
{
    selected_weapon = std::nullopt;
}

const ConfigState& get_config_state()
{
    return config_state;
}

bool configure_for_map(const std::string_view map_name)
{
    config_state = ConfigState();

    std::printf("configuring bot for map: %.*s\n",
                static_cast<int>(map_name.size()), map_name.data());

    static const std::string config_prefix    = "simulacrum/config/";
    static const std::string config_extension = ".txt";

    const std::string config_files[] = {
        config_prefix + "default" + config_extension,
        config_prefix + std::string(map_name) + config_extension
    };

    for (const std::string& fp : config_files) {
        std::printf("simulacrum: loading config file \"%s\"\n", fp.c_str());
        const bool success = sentinel_ExecuteConfigFile(fp.c_str());
        std::printf("simulacrum: config load %s\n", success ? "success" : "failure");
    }

    return true;
}

bool load()
{
    using sentutil::globals::get_command_line;
    using sentutil::script::install_script_function;

    if (const char* name = nullptr; get_command_line("-name", name) && name)
        set_name(name);

    if (const char* index = nullptr; get_command_line("-cindex", index) && index)
        config_state.persistent.client_index = std::clamp(std::atoi(index), 1, 16) - 1;

    if (get_command_line("-obfuscate-name"))
        obfuscate_name();

    return
        install_script_function<"simulacrum_config_weapon">(
            +[] (std::string_view tag_name) -> bool {
                if (tag_name == "!default!") {
                    config_state.select_default_weapon();
                    return true;
                }

                auto* tag = sentutil::tag::get_tag_meta_data(sentinel::make_signature("weap"), tag_name.data());
                if (tag) {
                    config_state.select_weapon(sentinel::identity<sentinel::tags::weapon>{tag->identity.raw});
                } else {
                    config_state.deselect_weapon();
                    sentutil::console::cprintf(sentutil::color::red, "no weapon tag \"%s\"", tag_name.data());
                }
                return static_cast<bool>(tag);
            },
            "sets the weapon to configure (with simulacrum_config_weapon_* commands); deselects if the tag is not found") &&
        install_config_field_accessors();

}

void reset()
{
    const auto old_persistent = config_state.persistent;
    const auto old_aim_config = config_state.aim_config;
    config_state = ConfigState();
    config_state.persistent = old_persistent;
    config_state.aim_config = old_aim_config;
}

} } // namespace simulacrum::config

namespace {

template<typename T>
struct remove_member_pointer { using type = T; };

template<class C, typename T>
struct remove_member_pointer<T C::*> { using type = T; };

template<sentutil::utility::static_string Name, auto MemberPointer>
struct ConfigFieldDescriptor {
    static constexpr auto field_name    = Name;
    static constexpr auto field_pointer = MemberPointer;
    using field_type = typename remove_member_pointer<decltype(MemberPointer)>::type;

    const char* field_usage;
};

bool install_config_field_accessors()
{
    using sentutil::utility::static_string;
    using sentutil::script::install_script_function;
    using simulacrum::config::AimConfig;
    using simulacrum::config::WeaponConfig;

#define MAKE_CONFIG_FIELD(ClassType, field, usage) ConfigFieldDescriptor<#field, &ClassType::field>{usage}
    static constexpr std::tuple weapon_config_fields = std::make_tuple(
        MAKE_CONFIG_FIELD(WeaponConfig, firing_interval,      "ticks between primary trigger pulses for the selected weapon"),
        MAKE_CONFIG_FIELD(WeaponConfig, preferred_zoom_level, "desired zoom level for the selected weapon")
    );

    static constexpr std::tuple aim_config_fields = std::make_tuple(
        MAKE_CONFIG_FIELD(AimConfig, lead_amount,        "the number of ticks to lead targets by"),
        MAKE_CONFIG_FIELD(AimConfig, turn_decay_rate,    "determines how fast the bot turns"),
        MAKE_CONFIG_FIELD(AimConfig, turn_constant_rate, "additional turn rate in radians per second"),
        MAKE_CONFIG_FIELD(AimConfig, fire_angle,         "the angle (in radians) to the target on which the bot may fire"),
        MAKE_CONFIG_FIELD(AimConfig, snap_angle,         "the angle (in radians) to the target on which the bot may snap")
    );
#undef MAKE_CONFIG_FIELD

    static constexpr static_string configure_weapon_prefix("simulacrum_config_weapon_");
    static constexpr static_string configure_aiming_prefix("simulacrum_config_aiming_");

    auto install_weapon_config_field = [] (const auto& fd) {
        using descriptor_type = std::decay_t<decltype(fd)>;
        using field_type      = typename descriptor_type::field_type;
        constexpr auto command_name  = configure_weapon_prefix + fd.field_name;
        constexpr auto field_pointer = fd.field_pointer;
        constexpr auto field_accessor = +[] (field_type value) -> void {
            if (config_state.selected_weapon) {
                sentinel::identity weapon_tag_id = config_state.selected_weapon.value();
                WeaponConfig& config = config_state.get_weapon_config(weapon_tag_id);
                config.*field_pointer = value;
            }
        };
        return install_script_function<command_name>(field_accessor, fd.field_usage);
    };

    auto install_aiming_config_field = [] (const auto& fd) {
        using descriptor_type = std::decay_t<decltype(fd)>;
        using field_type      = typename descriptor_type::field_type;
        constexpr auto command_name  = configure_aiming_prefix + fd.field_name;
        constexpr auto field_pointer = fd.field_pointer;

        constexpr auto field_accessor = +[] (field_type value) -> void {
            config_state.aim_config.*field_pointer = value;
        };
        return install_script_function<command_name>(field_accessor, fd.field_usage);
    };

    auto install_fields = [] (auto& installer, auto& fields) { return std::apply([&installer] (const auto&... fd) { return (installer(fd) && ...); }, fields); };

    return install_fields(install_weapon_config_field, weapon_config_fields)
        && install_fields(install_aiming_config_field, aim_config_fields);
}

void set_name(const char* name)
{
    if (!name)
        return;

    sentinel::profile_user_name_type& alias = *sentutil::globals::profile_user_name;
    const char* name_end = name + std::min(std::strlen(name), static_cast<std::size_t>(12));
    auto widen = [] (const char& c) -> wchar_t { return static_cast<wchar_t>(c); };

    std::transform(name, name_end, +alias.name, widen);
    alias.name[name_end - name] = L'\0';
}

void obfuscate_name()
{
    constexpr wchar_t zws  = L'\x200B'; // zero-width space
    constexpr wchar_t zwnj = L'\x200C'; // zero-width non-joiner
    constexpr wchar_t zwj  = L'\x200D'; // zero-width joiner

    std::wstring name = sentutil::globals::profile_user_name->name;

    // Strategy:
    // A string of length N < 11 has N + 1 insertion points.
    // We insert ZWJ and ZWNJ into these insertion points, providing us 2*(N+1)+1 variations.
    // Then, if we need more variations, we pad the end with a ZWS.

    const auto client_index = static_cast<unsigned>(config_state.persistent.client_index);
    name.reserve(12);
    while (client_index >= (2 * (name.size() + 1)))
        name.push_back(zws);

    if (name.size() >= 11) // max name length is 11, cannot insert for variations
        return;

    const auto insertion_it = name.cbegin() + (client_index <= name.size() ? client_index : client_index - name.size());
    const wchar_t insertion_character = client_index <= name.size() ? zwnj : zwj;
    name.insert(insertion_it, insertion_character);

    std::wcscpy(sentutil::globals::profile_user_name->name, name.c_str());
}

} // namespace (anonymous)
