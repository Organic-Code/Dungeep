#ifndef DUNGEEP_RESOURCE_MANAGER_HPP
#define DUNGEEP_RESOURCE_MANAGER_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018-2019, Lucas Lazare                                                                                                ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  		files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,  ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  		is furnished to do so, subject to the following conditions:                                                                 ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,        ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <utility>
#include <vector>
#include <string_view>
#include <json/json.h>
#include <unordered_map>
#include <memory>
#include <string>
#include <variant>
#include <spdlog/spdlog.h>
#include <array>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "environment/map.hpp"

namespace sf {
	class Sprite;
}

// For all functions in there, it is assumed that std::string_view are pointing either to a const char * or to an std::string,
// therefore std::string_view{}.data() shall always end by '\0' when passed to any of the methods
class resources {
	static constexpr unsigned int direction_count = static_cast<unsigned>(dungeep::direction::none);
	static constexpr unsigned int tiles_count = static_cast<unsigned>(tiles::none);

public:

	struct chest_count {
		unsigned short min;
		unsigned short max;
	};

	struct map_info {
		map::size_type size;
		std::vector<room_gen_properties> rooms_props;
		hallway_gen_properties hallways_props;
		chest_count rubbish_chest;
		chest_count wooden_chest;
		chest_count iron_chest;
		chest_count magic_chest;
	};

	struct creature_info {
		struct player_stats {  // human player only
			unsigned int mana;
			unsigned int mana_per_level;
			unsigned int magic_power;
			unsigned int magic_power_per_level;
		};

		struct creep_stats { // creep only
			unsigned int spawn_count;
			unsigned int spawn_interval;
			unsigned int spawn_long_interval;
		};

		struct creep_boss_stats { // creep boss only
			// to be defined
		};

		std::string_view name;
		dungeep::dim_uc size{0, 0};

		unsigned int base_hp;
		unsigned int hp_per_level;
		unsigned int base_physical_power;
		unsigned int physical_power_per_level;
		unsigned int base_armor;
		unsigned int armor_per_level;
		unsigned int base_resist;
		unsigned int resist_per_level;
		unsigned int base_crit_chance;
		unsigned int crit_chance_per_level;
		unsigned int base_move_speed;
		unsigned int move_speed_per_level;

		std::variant<player_stats, creep_stats, creep_boss_stats> other;
	};

	// map related info for creatures
	struct mob_map_rinfo {
		unsigned short min_level{0};
		unsigned short max_level{std::numeric_limits<unsigned short>::max()};
		unsigned short populate_factor{0}; // chance for a given mob to be of a type M is M.populate_factor / sum of all populate_factors on this level
	};

	inline static std::unique_ptr<resources> manager{};

	struct resource_acquisition_error : std::runtime_error {
		explicit resource_acquisition_error(const char* err) : std::runtime_error("Failed to load resource file " + std::string(err)) {}
		explicit resource_acquisition_error(const std::string& err) : std::runtime_error("Failed to load resource file " + err) {}
		explicit resource_acquisition_error(std::string_view err) : std::runtime_error("Failed to load resource file " + std::string(err)) {}
	};

	struct no_such_resource_error : std::runtime_error {
		explicit no_such_resource_error(const char* err) : std::runtime_error("Failed to load resource file " + std::string(err)) {}
		explicit no_such_resource_error(const std::string& err) : std::runtime_error("Failed to load resource file " + err) {}
		explicit no_such_resource_error(std::string_view err) : std::runtime_error("Failed to load resource file " + std::string(err)) {}
	};

	struct malformed_json_error : std::runtime_error {
		explicit malformed_json_error(const char* err) : std::runtime_error("Failed to load resource file " + std::string(err)) {}
		explicit malformed_json_error(const std::string& err) : std::runtime_error("Failed to load resource file " + err) {}
		explicit malformed_json_error(std::string_view err) : std::runtime_error("Failed to load resource file " + std::string(err)) {}
	};

	resources() noexcept;

	const std::unordered_map<std::string, map_info>& get_map_list() const {
		return maps;
	}

	const map_info& get_map(std::string_view map_name) const;

	void save_map(std::string_view map_name, const map_info& map_info);

	std::array<sf::Sprite, direction_count>& get_creature_sprite(std::string_view str) {
		return get_creature_sprite(std::string{str});
	}

	std::array<sf::Sprite, direction_count>& get_creature_sprite(const std::string& name) {
		return creatures_sprites[name];
	}

	std::array<sf::Sprite, tiles_count>& get_map_sprite(const std::string& name) {
		return maps_sprites[name];
	}

	unsigned int get_creature_count() const {
		return static_cast<unsigned int>(creatures_name_list.size());
	}

	const creature_info& read_creature(std::string_view name) const {
		try {
			return creatures.at(name);
		} catch (const std::out_of_range&) {
			spdlog::error("Tried to read info of creature '{}', which is not in the creature dataset. [internal error]", name);
			throw;
		}
	}

	unsigned int get_item_count() const {
		return items_json.size();
	}

	const Json::Value& read_item(unsigned int index) const {
		return items_json[index];
	}

	const Json::Value& read_item(const std::string& name) const {
		return items_json[name];
	}

	std::vector<std::pair<std::string_view, mob_map_rinfo>>
	get_creatures_for_level(unsigned int level, std::string_view map_name) const noexcept;

	std::string_view get_text(const std::string& key) const {
		try {
			return text_list.at(key);
		} catch (std::out_of_range&) {
			spdlog::warn("Missing text. Key: '{}'.", key);
			return empty_string;
		}
	}

private:

	void load_config() noexcept;
	void load_maps() noexcept;
	void load_creatures() noexcept;
	void load_sprites() noexcept;
	void load_items() noexcept;
	void load_translations() noexcept;

	void parse_creatures() noexcept;
	void parse_maps() noexcept;

	void load_creature_sprites(const std::string& name, const Json::Value& values) noexcept;
	void load_map_sprites(const std::string& name, const Json::Value& values) noexcept;

	inline static const std::string empty_string{};

	Json::Value config{};

	Json::Value maps_json{};
	std::unordered_map<std::string, map_info> maps{};

	Json::Value items_json{}; // TODO: items_parsed

	std::unordered_map<std::string, std::string> text_list{};

	Json::Value creatures_json{};
	std::vector<std::string> creatures_name_list{};
	std::unordered_map<std::string_view, std::unordered_map<std::string_view, mob_map_rinfo>> creatures_info_per_map{};
	std::unordered_map<std::string_view, creature_info> creatures{};

	sf::Texture texture{};
	std::unordered_map<std::string, std::array<sf::Sprite, direction_count>> creatures_sprites{};
	std::unordered_map<std::string, std::array<sf::Sprite, tiles_count>> maps_sprites{};
};

#endif //DUNGEEP_RESOURCE_MANAGER_HPP
