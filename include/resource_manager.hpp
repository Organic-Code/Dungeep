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

#include "map.hpp"

namespace sf {
	class Sprite;
}

class resources {
	static constexpr unsigned int direction_count = static_cast<unsigned>(dungeep::direction::none);
	static constexpr unsigned int tiles_count = static_cast<unsigned>(tiles::none);

public:
	static resources manager;

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

	const Json::Value& read_creature(const std::string& name) const {
		return creatures[name];
	}

private:

	void load_maps() noexcept;
	void load_creatures() noexcept;
	void load_sprites() noexcept;

	void load_creature_sprites(const std::string& name, const Json::Value& values) noexcept;
	void load_map_sprites(const std::string& name, const Json::Value& values) noexcept;

	Json::Value maps{};
	std::vector<std::string> map_list{};

	Json::Value creatures{};

	sf::Texture texture{};
	std::unordered_map<std::string, std::array<sf::Sprite, direction_count>> creatures_sprites{};
	std::unordered_map<std::string, std::array<sf::Sprite, tiles_count>> maps_sprites{};
};

#endif //DUNGEEP_RESOURCE_MANAGER_HPP
