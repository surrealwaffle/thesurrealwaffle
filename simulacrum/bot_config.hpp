#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>

#include <sentinel/structures/weapon.hpp>
#include <sentinel/tags/weapon.hpp>
#include <sentutil/constants.hpp>

namespace simulacrum { namespace config {

struct WeaponConfig {
    long  firing_interval;
    short preferred_zoom_level;
};

struct AimConfig {
    long  lead_amount; ///< The amount of ticks to lead targets by.
    float turn_decay_rate;    ///< The factor applied to the decaying part of the
                              ///< turning model differential equation.
    float turn_constant_rate; ///< The number of radians per second permitted by
                              ///< the turning model in excess of the decaying part.


    float fire_angle; ///< The angle to the target on which the bot may fire.
    float snap_angle; ///< The angle to the target on which the bot may snap.
};

struct ConfigState {
    using weapon_tag = sentinel::tags::weapon;
    using weapon_tag_id = sentinel::identity<weapon_tag>;

    ConfigState()
        :  persistent {
            .client_index = 0
        }, aim_config {
            .lead_amount        = 0L,
            .turn_decay_rate    = 8.0f,
            .turn_constant_rate = sentutil::constants::pi / 180,
            .fire_angle         = sentutil::constants::pi / 90,
            .snap_angle         = sentutil::constants::pi / 270
        }, selected_weapon(std::nullopt)
        ,  default_weapon_config{
            .firing_interval      = 5L,
            .preferred_zoom_level = -1
        }, weapon_configs() { }

    struct {
        int client_index;
    } persistent;

    AimConfig aim_config;

    /** \brief The current weapon selected for configuration.
     *
     * If this optional is empty, then no weapon is currently selected for
     * configuration.
     */
    std::optional<weapon_tag_id> selected_weapon;

    /** \brief The weapon configuration used for unconfigured weapons.
     */
    WeaponConfig default_weapon_config = {
        .firing_interval      = 5L,
        .preferred_zoom_level = -1
    };

    /** \brief Stores the explicit weapon configurations.
     */
    std::unordered_map<sentinel::identity_raw, WeaponConfig> weapon_configs;

    /** \brief Retrieves a weapon's configuration.
     *
     * \return The configuration for the given weapon by \a id, or
     *         the #default_weapon_config if the weapon is unconfigured.
     */
    WeaponConfig& get_weapon_config(const weapon_tag_id& id);
    WeaponConfig& get_weapon_config(const sentinel::identity<sentinel::weapon>& weapon_id);

    /** \brief Retrieves a weapon's configuration.
     *
     * \return The configuration for the given weapon by \a id, or
     *         the #default_weapon_config if the weapon is unconfigured.
     */
    const WeaponConfig& get_weapon_config(const weapon_tag_id& id) const;
    const WeaponConfig& get_weapon_config(const sentinel::identity<sentinel::weapon>& weapon_id) const;

    /** \brief Selects a weapon for configuration.
     *
     * If \a weapon has no associated configuration, it is initialized to the
     * #default_weapon_config.
     */
    void select_weapon(const weapon_tag_id& weapon);

    /** \brief Selects the default weapon configuration.
     */
    void select_default_weapon();

    /** \brief Deselects the current weapon for configuration.
     */
    void deselect_weapon();
};

const ConfigState& get_config_state();

/** \brief Loads the bot configuration files for the supplied map.
 */
bool configure_for_map(std::string_view map_name);

/** \brief Installs `simulacrum` configuration commands.
 */
bool load();

/** \brief Resets the entire configuration state.
 */
void reset();

} } // namespace simulacrum::config
