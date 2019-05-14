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

#include <string_view>
#include <fstream>
#include <algorithm>
#include <SFML/Graphics/Sprite.hpp>
#include <utils/resource_keys.hpp>
#include <spdlog/spdlog.h>

#include "utils/resource_manager.hpp"


#pragma GCC diagnostic push
// I know this doesn't work, but letting it there anyway
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"

#pragma clang diagnostic ignored "-Wexceptions"
#pragma GCC diagnostic ignored "-Wterminate"

namespace {
	constexpr bool disable_resources = false; // everything might not work if disabled.

	// Merges Json::Value by simply over-writing top level keys
	[[maybe_unused]] void simple_merge(Json::Value& val1, Json::Value&& val2) {
		std::vector<std::string> members = val2.getMemberNames();
		for (const std::string& member : members) {
			val1[member] = std::move(val2[member]);
		}
	}

	// Merges Json::Value more deeply (merges arrays/lists instead of over-writing them)
	[[maybe_unused]] void recursive_merge(Json::Value& val1, Json::Value&& val2) {
		std::vector<std::string> members = val2.getMemberNames();
		for (const std::string& member : members) {
			Json::Value& child = val2[member];
			if (child.type() == Json::ValueType::arrayValue || child.type() == Json::ValueType::objectValue) {
				recursive_merge(val1[member], std::move(child));
			} else {
				val1[member] = std::move(child);
			}
		}
	}

	std::ifstream try_open(std::string_view path) {
		std::ifstream file(path.data(), std::ios_base::in);
		if (!file) {
			throw resources::resource_acquisition_error(path);
		}
		return file;
	}
}

namespace paths {
	namespace {
#define RESOURCE_ROOT_FOLDER "resources/"
#define RESOURCE_LANG_FOLDER RESOURCE_ROOT_FOLDER"lang/"
		[[maybe_unused]] constexpr std::string_view resource_folder = RESOURCE_ROOT_FOLDER;
		constexpr std::string_view lang_folder = RESOURCE_LANG_FOLDER;
		constexpr std::string_view map_file = RESOURCE_ROOT_FOLDER"maps.json";
		constexpr std::string_view creatures_file = RESOURCE_ROOT_FOLDER"creatures.json";
		constexpr std::string_view sprites_file = RESOURCE_ROOT_FOLDER"sprites.json";
		constexpr std::string_view texture_file = RESOURCE_ROOT_FOLDER"texture.png";
		constexpr std::string_view items_file = RESOURCE_ROOT_FOLDER"items.json";
		constexpr std::string_view default_lang_file = RESOURCE_LANG_FOLDER"default_language.json";
		constexpr std::string_view config_file = RESOURCE_ROOT_FOLDER"config.json";
#undef RESOURCE_LANG_FOLDER
#undef RESOURCE_ROOT_FOLDER
	}
}

namespace {
	namespace map_tags {
		namespace categories {
			constexpr const char * zones = "zones properties";
			constexpr const char * rooms = "rooms properties";
			constexpr const char * holes = "holes properties";
			constexpr const char * halls = "halls properties";
		}
		namespace sizes {
			constexpr const char * width = "width";
			constexpr const char * height = "height";
		}
		namespace zones {
			constexpr const char * avg_size = "average size";
			constexpr const char * size_deviation = "size deviation";
			constexpr const char * borders_fuzzinness = "borders fuzziness";
			constexpr const char * borders_fuzzy_deviation = "borders fuzzy deviation";
			constexpr const char * borders_fuzzy_distance = "borders fuzzy distance";
			constexpr const char * min_height = "minimum height";
			constexpr const char * max_height = "maximum height";
			constexpr const char * avg_rooms_n = "average room count";
			constexpr const char * rooms_n_dev = "room count deviation";
			constexpr const char * avg_holes_n = "average hole count";
			constexpr const char * holes_n_dev = "hole count deviation";
		}
		namespace halls {
			constexpr const char * curliness = "curliness";
			constexpr const char * curly_min_distance = "curly minimum distance";
			constexpr const char * curly_segment_avg_size = "curly segment average size";
			constexpr const char * curly_segment_size_dev = "curly segment size deviation";
			constexpr const char * avg_width = "average width";
			constexpr const char * width_dev = "width deviation";
			constexpr const char * min_width = "minimum width";
			constexpr const char * max_width = "maximum width";
		}
	}
}

resources::resources() noexcept {
	if constexpr (disable_resources) {
		return;
	}
	spdlog::info("Loading resources.");
	load_config();
	load_maps();
	load_creatures();
	load_sprites();
	load_items();
	load_translations();
	load_creature_infos();
	spdlog::info("Resources loading: done.");
}


const std::vector<std::string>& resources::get_map_list() const {
	return map_list;
}

std::tuple<map::size_type, std::vector<room_gen_properties>, hallway_gen_properties>
resources::get_map(std::string_view map_name) const {
	namespace tags = map_tags;

	if constexpr (disable_resources) {
		return {};
	}

	const Json::Value& map = maps[map_name.data()];

	std::tuple<map::size_type, std::vector<room_gen_properties>, hallway_gen_properties> ans;
	auto& [size, rooms_props, hallways_prop] = ans;

	size.width = map[tags::sizes::width].asUInt();
	size.height = map[tags::sizes::height].asUInt();

	for (const Json::Value& zone : map[tags::categories::zones]) {
		const Json::Value& rooms_properties = zone[tags::categories::rooms];
		const Json::Value& holes_properties = zone[tags::categories::holes];
		rooms_props.push_back(
				{
					{
						rooms_properties[tags::zones::avg_size].asFloat(),
						rooms_properties[tags::zones::size_deviation].asFloat(),
						rooms_properties[tags::zones::borders_fuzzinness].asFloat(),
						rooms_properties[tags::zones::borders_fuzzy_deviation].asFloat(),
						rooms_properties[tags::zones::borders_fuzzy_distance].asUInt(),
						rooms_properties[tags::zones::min_height].asUInt(),
						rooms_properties[tags::zones::max_height].asUInt()
					},
					{
						holes_properties[tags::zones::avg_size].asFloat(),
						holes_properties[tags::zones::size_deviation].asFloat(),
						holes_properties[tags::zones::borders_fuzzinness].asFloat(),
						holes_properties[tags::zones::borders_fuzzy_deviation].asFloat(),
						holes_properties[tags::zones::borders_fuzzy_distance].asUInt(),
						holes_properties[tags::zones::min_height].asUInt(),
						holes_properties[tags::zones::max_height].asUInt()
					},
					zone[tags::zones::avg_rooms_n].asFloat(),
					zone[tags::zones::rooms_n_dev].asFloat(),
					zone[tags::zones::avg_holes_n].asFloat(),
					zone[tags::zones::holes_n_dev].asFloat()
				}
		);
	}

	const Json::Value& halls_properties = map[tags::categories::halls];
	hallways_prop.curliness = halls_properties[tags::halls::curliness].asFloat();
	hallways_prop.curly_min_distance = halls_properties[tags::halls::curly_min_distance].asFloat();
	hallways_prop.curly_segment_avg_size = halls_properties[tags::halls::curly_segment_avg_size].asFloat();
	hallways_prop.curly_segment_size_dev = halls_properties[tags::halls::curly_segment_size_dev].asFloat();
	hallways_prop.avg_width = halls_properties[tags::halls::avg_width].asFloat();
	hallways_prop.width_dev = halls_properties[tags::halls::width_dev].asFloat();
	hallways_prop.min_width = halls_properties[tags::halls::min_width].asUInt();
	hallways_prop.max_width = halls_properties[tags::halls::max_width].asUInt();

	return ans;
}

void resources::save_map(std::string_view map_name, map::size_type size
		, const std::vector<room_gen_properties>& room_properties, const hallway_gen_properties& halls_properties) {
	namespace tags = map_tags;

	if constexpr (disable_resources) {
		return;
	}

	Json::Value& map = maps[map_name.data()];
	map[tags::sizes::width] = size.width;
	map[tags::sizes::height] = size.height;

	Json::Value& zone = map[tags::categories::zones];
	unsigned int i = 0;
	for (const room_gen_properties& gen_prop : room_properties) {
		Json::Value& i_zone = zone[i++];
		Json::Value& rooms_properties = i_zone[tags::categories::rooms];
		Json::Value& holes_properties = i_zone[tags::categories::holes];
		rooms_properties[tags::zones::avg_size] = static_cast<double>(gen_prop.rooms_properties.avg_size);
		rooms_properties[tags::zones::size_deviation] = static_cast<double>(gen_prop.rooms_properties.size_deviation);
		rooms_properties[tags::zones::borders_fuzzinness] = static_cast<double>(gen_prop.rooms_properties.borders_fuzzinness);
		rooms_properties[tags::zones::borders_fuzzy_deviation] = static_cast<double>(gen_prop.rooms_properties.borders_fuzzy_deviation);
		rooms_properties[tags::zones::borders_fuzzy_distance] = gen_prop.rooms_properties.borders_fuzzy_distance;
		rooms_properties[tags::zones::min_height] = gen_prop.rooms_properties.min_height;
		rooms_properties[tags::zones::max_height] = gen_prop.rooms_properties.max_height;
		holes_properties[tags::zones::avg_size] = static_cast<double>(gen_prop.holes_properties.avg_size);
		holes_properties[tags::zones::size_deviation] = static_cast<double>(gen_prop.holes_properties.size_deviation);
		holes_properties[tags::zones::borders_fuzzinness] = static_cast<double>(gen_prop.holes_properties.borders_fuzzinness);
		holes_properties[tags::zones::borders_fuzzy_deviation] = static_cast<double>(gen_prop.holes_properties.borders_fuzzy_deviation);
		holes_properties[tags::zones::borders_fuzzy_distance] = gen_prop.holes_properties.borders_fuzzy_distance;
		holes_properties[tags::zones::min_height] = gen_prop.holes_properties.min_height;
		holes_properties[tags::zones::max_height] = gen_prop.holes_properties.max_height;
		i_zone[tags::zones::avg_rooms_n] = static_cast<double>(gen_prop.avg_rooms_n);
		i_zone[tags::zones::rooms_n_dev] = static_cast<double>(gen_prop.rooms_n_dev);
		i_zone[tags::zones::avg_holes_n] = static_cast<double>(gen_prop.avg_holes_n);
		i_zone[tags::zones::holes_n_dev] = static_cast<double>(gen_prop.holes_n_dev);
	}

	Json::Value& halls = map[tags::categories::halls];
	halls[tags::halls::curliness] = static_cast<double>(halls_properties.curliness);
	halls[tags::halls::curly_min_distance] = static_cast<double>(halls_properties.curly_min_distance);
	halls[tags::halls::curly_segment_avg_size] = static_cast<double>(halls_properties.curly_segment_avg_size);
	halls[tags::halls::curly_segment_size_dev] = static_cast<double>(halls_properties.curly_segment_size_dev);
	halls[tags::halls::avg_width] = static_cast<double>(halls_properties.avg_width);
	halls[tags::halls::width_dev] = static_cast<double>(halls_properties.width_dev);
	halls[tags::halls::min_width] = halls_properties.min_width;
	halls[tags::halls::max_width] = halls_properties.max_width;

	if (std::find(map_list.begin(), map_list.end(), map_name) == map_list.end()) {
		map_list.emplace_back(map_name);
	}

	std::ofstream map_file(paths::map_file.data(), std::ios_base::trunc);
	map_file << maps;
}

void resources::load_config() noexcept {
	spdlog::debug("Loading config file: {}", paths::config_file);
	std::ifstream config_file = try_open(paths::config_file);
	config_file >> config;
}

void resources::load_maps() noexcept {
	spdlog::debug("Loading config maps: {}", paths::map_file);
	std::ifstream maps_file = try_open(paths::map_file);
	maps_file >> maps;
	map_list = maps.getMemberNames();
}

void resources::load_creatures() noexcept {
	spdlog::debug("Loading creatures stats: {}", paths::creatures_file);
	std::ifstream creatures_file = try_open(paths::creatures_file);
	creatures_file >> creatures;
}

void resources::load_sprites() noexcept {
	spdlog::debug("Loading sprites and textures: {} :: {}", paths::sprites_file, paths::texture_file);
	std::ifstream sprites_file = try_open(paths::sprites_file);

	if (!texture.loadFromFile(std::string(paths::texture_file))) {
		throw resource_acquisition_error(paths::texture_file);
	}
	texture.setSmooth(false);

	Json::Value val;
	sprites_file >> val;
	sprites_file.close();

	const Json::Value& creatures_json = val["creatures"];
	std::vector<std::string> members = creatures_json.getMemberNames();
	for (const std::string& member : members) {
		load_creature_sprites(member, creatures_json[member]);
	}


	const Json::Value& map_json = val["maps"];
	members = map_json.getMemberNames();
	for (const std::string& member : members) {
		load_map_sprites(member, map_json[member]);
	}
}

void resources::load_items() noexcept {
	spdlog::debug("Loading items stats: {}", paths::items_file);
	std::ifstream items_file = try_open(paths::items_file);
	items_file >> items;
	std::vector<std::string> members = items.getMemberNames();
	for (const std::string& member : members) {
		items[member][keys::item::name] = member;
	}
}

void resources::load_translations() noexcept {
	spdlog::debug("Loading text strings: {}", paths::default_lang_file);
	std::ifstream default_lang_file = try_open(paths::default_lang_file);
	default_lang_file >> text_list_json;

	if (config.isMember(keys::config::language)) {
		std::string lang = config[keys::config::language].asString();
		std::string lang_file = paths::lang_folder.data() + std::move(lang);
		spdlog::debug("Loading translations: {}", lang_file);
		std::ifstream translations_file(lang_file, std::ios_base::in);

		if (translations_file) {
			Json::Value translation;
			translations_file >> translation;
			simple_merge(text_list_json, std::move(translation));
		}
	}
}

void resources::load_creature_infos() noexcept {
	spdlog::debug("Pre-parsing creatures infos");
	creatures_name_list = creatures.getMemberNames();
	for (const std::string& name : creatures_name_list) {
		Json::Value& current_creature = creatures[name];
		if (current_creature[keys::creature::type].asString() != values::creature::type::player) {
			const sf::IntRect& sprite_rect = creatures_sprites[name][0].getTextureRect();

			creature_info inf{name};
			inf.size.x = static_cast<std::uint8_t>(sprite_rect.width);
			inf.size.y = static_cast<std::uint8_t>(sprite_rect.height);

			Json::Value& maps_ = current_creature[keys::creature::map::list];
			std::vector<std::string> map_names = maps_.getMemberNames();
			for (const std::string& map : map_names) {
				Json::Value& current_map = maps_[map];

				if (current_map.isMember(keys::creature::map::min_level)) {
					inf.min_level = static_cast<unsigned short>(current_map[keys::creature::map::min_level].asUInt());
				} else {
					inf.min_level = 0u;
				}

				if (current_map.isMember(keys::creature::map::max_level)) {
					inf.max_level = static_cast<unsigned short>(current_map[keys::creature::map::max_level].asUInt());
				} else {
					inf.max_level = std::numeric_limits<unsigned short>::max();
				}

				if (current_map.isMember(keys::creature::map::populate_factor)) {
					inf.populate_factor = static_cast<unsigned short>(current_map[keys::creature::map::populate_factor].asUInt());
				} else {
					inf.populate_factor = 1;
				}

				creatures_info_per_map[map].push_back(inf);
			}
		}
	}
}

void resources::load_creature_sprites(const std::string& name, const Json::Value& values) noexcept {
	spdlog::trace("> loading creature sprite: {}", name);

	sf::Texture& the_texture = texture;

	auto load_part = [&the_texture](const Json::Value& sub_value) -> sf::Sprite {
		sf::Sprite spr;
		spr.setTexture(the_texture);
		spr.setTextureRect(sf::IntRect(
				sub_value["x"].asInt()
				, sub_value["y"].asInt()
				, sub_value["width"].asInt()
				, sub_value["height"].asInt())
		);
		return spr;
	};

	auto get_or_flip = [&values, &load_part](std::string_view name_, const sf::Sprite& spr, float xscale, float yscale) -> sf::Sprite {
		sf::Sprite ans;
		if (values.isMember(name_.data())) {
			ans = load_part(values[name_.data()]);
		} else {
			ans = spr;
			ans.scale(xscale, yscale);
		}
		return ans;
	};

	sf::Sprite top       = load_part(values["top"]);
	sf::Sprite top_right = load_part(values["top right"]);
	sf::Sprite right     = load_part(values["right"]);

	sf::Sprite bot_right = get_or_flip("bot right", top_right,  1.f, -1.f); // flip around vertical axis
	sf::Sprite bot       = get_or_flip("bot"      , top      ,  1.f, -1.f);
	sf::Sprite bot_left  = get_or_flip("bot left" , bot_right, -1.f,  1.f); // flip around hztal axis
	sf::Sprite left      = get_or_flip("left"     , right    , -1.f,  1.f);
	sf::Sprite top_left  = get_or_flip("top left" , top_right, -1.f,  1.f);

	using dungeep::direction;
	auto& array = creatures_sprites.emplace(std::make_pair(name, std::array<sf::Sprite, direction_count>())).first->second;
	array[static_cast<unsigned>(direction::top      )] = std::move(top);
	array[static_cast<unsigned>(direction::top_right)] = std::move(top_right);
	array[static_cast<unsigned>(direction::right    )] = std::move(right);
	array[static_cast<unsigned>(direction::bot_right)] = std::move(bot_right);
	array[static_cast<unsigned>(direction::bot      )] = std::move(bot);
	array[static_cast<unsigned>(direction::bot_left )] = std::move(bot_left);
	array[static_cast<unsigned>(direction::left     )] = std::move(left);
	array[static_cast<unsigned>(direction::top_left )] = std::move(top_left);
}

void resources::load_map_sprites(const std::string& name, const Json::Value& values) noexcept {
	spdlog::trace("> loading map sprite: {}", name);
	sf::Texture& the_texture = texture;

	auto load_part = [&the_texture](const Json::Value& sub_value) -> sf::Sprite {
		sf::Sprite spr;
		spr.setTexture(the_texture);
		spr.setTextureRect(sf::IntRect(
				sub_value["x"].asInt()
				, sub_value["y"].asInt()
				, sub_value["width"].asInt()
				, sub_value["height"].asInt())
		);
		return spr;
	};

	auto& array = maps_sprites.emplace(std::make_pair(name, std::array<sf::Sprite, tiles_count>())).first->second;
	array[static_cast<unsigned>(tiles::hole)       ] = load_part(values["hole"]);
	array[static_cast<unsigned>(tiles::walkable)   ] = load_part(values["walkable"]);
	array[static_cast<unsigned>(tiles::empty_space)] = load_part(values["empty space"]);
	array[static_cast<unsigned>(tiles::wall)       ] = load_part(values["wall"]);

}

std::vector<resources::creature_info> resources::get_creatures_for_level(unsigned int level, const std::string& map_name) const noexcept {
	// Improvement: find an actual algorithm to do that ? \:
	spdlog::trace("Loading creatures list for map {}, at level {}.", map_name, level);
	std::vector<creature_info> creature_list;
	try {
		const std::vector<creature_info>& full_creature_list = creatures_info_per_map.at(map_name);
		for (const creature_info& creature : full_creature_list) {
			if (creature.min_level <= level && level <= creature.max_level) {
				creature_list.push_back(creature);
			}
		}
	} catch (const std::out_of_range&) {}
	spdlog::trace("Found {} creature(s)", creature_list.size());
	return creature_list;
}

#pragma clang diagnostic pop
#pragma GCC diagnostic pop
