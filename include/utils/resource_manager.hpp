#ifndef DUNGEEP_RESOURCE_MANAGER_HPP
#define DUNGEEP_RESOURCE_MANAGER_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018, Lucas Lazare                                                                                                     ///
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
#include <string>
#include <array>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "environment/map.hpp"

namespace sf {
	class Sprite;
}

class resources {
	static constexpr unsigned int direction_count = static_cast<unsigned>(dungeep::direction::none);
	static constexpr unsigned int tiles_count = static_cast<unsigned>(tiles::none);

public:
#include "resource_keys.hpp"

	static resources manager;

	struct resource_acquisition_error : std::runtime_error {
		explicit resource_acquisition_error(const char* err) : std::runtime_error("Failed to load resource file " + std::string(err)) {}
		explicit resource_acquisition_error(const std::string& err) : std::runtime_error("Failed to load resource file " + err) {}
		explicit resource_acquisition_error(std::string_view err) : std::runtime_error("Failed to load resource file " + std::string(err)) {}
	};

	resources() noexcept;

	const std::vector<std::string>& get_map_list() const;

	std::tuple<map::size_type, std::vector<room_gen_properties>, hallway_gen_properties> get_map(std::string_view map_name) const;

	void save_map(std::string_view map_name, map::size_type size, const std::vector<room_gen_properties>& room_properties, const hallway_gen_properties& halls_properties);

	std::array<sf::Sprite, direction_count>& get_creature_sprite(const std::string& name) {
		return creatures_sprites[name];
	}

	std::array<sf::Sprite, tiles_count>& get_map_sprite(const std::string& name) {
		return maps_sprites[name];
	}

	unsigned int get_creature_count() const {
		return creatures.size();
	}

	const Json::Value& read_creature(unsigned int index) const {
		return creatures[index];
	}

	const Json::Value& read_creature(const std::string& name) const {
		return creatures[name];
	}

	unsigned int get_item_count() const {
		return items.size();
	}

	const Json::Value& read_item(unsigned int index) const {
		return items[index];
	}

	const Json::Value& read_item(const std::string& name) const {
		return items[name];
	}

	const std::vector<std::pair<std::string, dungeep::dim_ui>>& get_creatures_for_level(unsigned int level) const noexcept;

	// constructs the string the first time it's needed, re-uses it afterward
	// does not invalidates previously fetched references
	const std::string& get_text(const char* key) {
		try {
			return text_list.at(key);
		} catch (std::out_of_range&) {
			if (text_list_json.isMember(key)) {
				std::string str = text_list_json[key].asString();
				text_list_json.removeMember(key);
				return text_list.emplace(std::pair(key, std::move(str))).first->second;
			} else {
				return empty_string;
			}
		}
	}

private:

	void load_config() noexcept;
	void load_maps() noexcept;
	void load_creatures() noexcept;
	void load_sprites() noexcept;
	void load_items() noexcept;
	void load_translations() noexcept;

	void load_creature_sprites(const std::string& name, const Json::Value& values) noexcept;
	void load_map_sprites(const std::string& name, const Json::Value& values) noexcept;

	inline static const std::string empty_string{};

	Json::Value config{};

	Json::Value maps{};
	std::vector<std::string> map_list{};

	Json::Value creatures{};

	Json::Value items{};

	Json::Value text_list_json{};
	std::unordered_map<const char*, std::string> text_list{};

	sf::Texture texture{};
	std::unordered_map<std::string, std::array<sf::Sprite, direction_count>> creatures_sprites{};
	std::unordered_map<std::string, std::array<sf::Sprite, tiles_count>> maps_sprites{};
};

#endif //DUNGEEP_RESOURCE_MANAGER_HPP
