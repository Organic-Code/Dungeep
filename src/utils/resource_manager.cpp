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
			spdlog::error("Failed to open file {}.", path);
			throw resources::resource_acquisition_error(path);
		}
		return file;
	}

	template <typename T>
	void try_assign(const Json::Value& val, const char* key, T& v, std::string_view data_source) {
		if (!val.isMember(key)) {
			spdlog::error("Trying to fetch {} from {}, but that key is missing.", key, data_source);
			throw resources::malformed_json_error("missing key: " + std::string(key) + "(in value " + std::string(data_source) + ")");
		}
		const Json::Value& actual_value = val[key];

		if constexpr (std::is_integral_v<T>) {
			if (!actual_value.isIntegral()) {
				spdlog::error("While fetching {} from {}: {} must be an integral type.", key, data_source, key);
				throw resources::malformed_json_error(std::string(key) + " must be an integral type");
			}

			std::conditional_t<std::numeric_limits<T>::is_signed, Json::LargestInt, Json::LargestUInt> integer_value;

			if constexpr (std::numeric_limits<T>::is_signed) {
				integer_value = actual_value.asLargestInt();
			} else {
				integer_value = actual_value.asLargestUInt();
			}

			if (integer_value > std::numeric_limits<T>::max()) {
				v = std::numeric_limits<T>::max();
				spdlog::warn("Value {} in {}: {} is too high. Clamping to {}", key, data_source, integer_value, v);

			} else if (integer_value  < std::numeric_limits<T>::min()) {
				v = std::numeric_limits<T>::min();
				spdlog::warn("Value {} in {}: {} is too low. Clamping to {}", key, data_source, integer_value, v);

			} else {
				v = static_cast<T>(integer_value);
			}


		} else if constexpr (std::is_same_v<T, std::string>) {
			v = actual_value.asString();

		} else if constexpr (std::is_same_v<T, float>) {
			if (!actual_value.isDouble()) {
				spdlog::error("While fetching {} from {}: {} must be a numeric type.", key, data_source, key);
				throw resources::malformed_json_error(std::string(key) + " must be a numeric type");
			}
			v = actual_value.asFloat();

		} else {
			static_assert(std::is_same_v<T, unsigned int>);
		}

	}

	template <typename T>
	T try_get_as(const Json::Value& val, const char* key, std::string_view data_source) {
		T v{};
		try_assign(val, key, v, data_source);
		return v;
	}

	const Json::Value& try_get(const Json::Value& val, const char* key, std::string_view data_source) {
		if (!val.isMember(key)) {
			spdlog::error("Trying to fetch {} from {}, but that key is missing.", key, data_source);
			throw resources::malformed_json_error("missing key: " + std::string(key) + "(in value " + std::string(data_source) + ")");
		}

		return val[key];
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

resources::resources() noexcept {
	spdlog::info("Loading resources.");
	load_config();

	load_creatures();
	load_maps();

	load_sprites();
	load_items();
	load_translations();
	parse_maps();
	parse_creatures(); // /!\ Must be done AFTER parse_map
	spdlog::info("Resources loading: done.");
}

const resources::map_info& resources::get_map(std::string_view map_name) const {

	auto it = maps.find(std::string{map_name});
	if (it != maps.end()) {
		return it->second;
	}

	spdlog::error("Tried to retrieve map {}, which is not in the map dataset. [internal error]", map_name);
	if (maps.empty()) {
		spdlog::error("The map dataset is empty. Aborting");
		throw no_such_resource_error("map " + std::string(map_name));
	}

	it = maps.begin();
	spdlog::error("Returning map {}.", it->first);
	return it->second;
}

void resources::save_map(std::string_view map_name, const map_info& new_map) {
	namespace tags = keys::map;

	Json::Value& map = maps_json[map_name.data()];
	map[tags::sizes::width] = new_map.size.width;
	map[tags::sizes::height] = new_map.size.height;

	Json::Value& zone = map[tags::categories::zones];
	unsigned int i = 0;
	for (const room_gen_properties& gen_prop : new_map.rooms_props) {
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
	halls[tags::halls::curliness] = static_cast<double>(new_map.hallways_props.curliness);
	halls[tags::halls::curly_min_distance] = static_cast<double>(new_map.hallways_props.curly_min_distance);
	halls[tags::halls::curly_segment_avg_size] = static_cast<double>(new_map.hallways_props.curly_segment_avg_size);
	halls[tags::halls::curly_segment_size_dev] = static_cast<double>(new_map.hallways_props.curly_segment_size_dev);
	halls[tags::halls::avg_width] = static_cast<double>(new_map.hallways_props.avg_width);
	halls[tags::halls::width_dev] = static_cast<double>(new_map.hallways_props.width_dev);
	halls[tags::halls::min_width] = new_map.hallways_props.min_width;
	halls[tags::halls::max_width] = new_map.hallways_props.max_width;

	map[tags::chests::rubbish_min] = new_map.rubbish_chest.min;
	map[tags::chests::rubbish_max] = new_map.rubbish_chest.max;
	map[tags::chests::wooden_min ] = new_map.wooden_chest.min;
	map[tags::chests::wooden_max ] = new_map.wooden_chest.max;
	map[tags::chests::magic_min  ] = new_map.magic_chest.min;
	map[tags::chests::magic_max  ] = new_map.magic_chest.max;
	map[tags::chests::iron_min   ] = new_map.iron_chest.min;
	map[tags::chests::iron_max   ] = new_map.iron_chest.max;

	std::ofstream map_file(paths::map_file.data(), std::ios_base::trunc);
	map_file << maps_json;
}

void resources::load_config() noexcept {
	spdlog::debug("Loading config file: {}", paths::config_file);
	std::ifstream config_file = try_open(paths::config_file);
	config_file >> config;
}

void resources::load_maps() noexcept {
	spdlog::debug("Loading config maps: {}", paths::map_file);
	std::ifstream maps_file = try_open(paths::map_file);
	maps_file >> maps_json;
}

void resources::load_creatures() noexcept {
	spdlog::debug("Loading creatures stats: {}", paths::creatures_file);
	std::ifstream creatures_file = try_open(paths::creatures_file);
	creatures_file >> creatures_json;
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

	const Json::Value& creatures_sprite_json = val[keys::sprites::creatures];
	std::vector<std::string> members = creatures_sprite_json.getMemberNames();
	for (const std::string& member : members) {
		load_creature_sprites(member, creatures_sprite_json[member]);
	}


	const Json::Value& map_json = val[keys::sprites::maps];
	members = map_json.getMemberNames();
	for (const std::string& member : members) {
		load_map_sprites(member, map_json[member]);
	}
}

void resources::load_items() noexcept {
	spdlog::debug("Loading items stats: {}", paths::items_file);
	std::ifstream items_file = try_open(paths::items_file);
	items_file >> items_json;
	std::vector<std::string> members = items_json.getMemberNames();
	for (const std::string& member : members) {
		items_json[member][keys::item::name] = member;
	}
}

void resources::load_translations() noexcept {
	spdlog::debug("Loading text strings: {}", paths::default_lang_file);
	std::ifstream default_lang_file = try_open(paths::default_lang_file);

	Json::Value text_list_json{};
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
		} else {
			spdlog::warn("Failed to open translation file {}. Bailing out.", lang_file);
		}
	}

	std::vector<std::string> keys = text_list_json.getMemberNames();
	for (std::string& key : keys) {
		text_list.emplace(std::move(key), text_list_json[key].asString());
	}
}

void resources::parse_creatures() noexcept {
	spdlog::debug("Parsing creatures infos");
	creatures_name_list = creatures_json.getMemberNames();

	for (const std::string& name : creatures_name_list) {
		spdlog::trace("> parsing creature {}", name);

		Json::Value& current_creature = creatures_json[name];
		if (current_creature[keys::creature::type].asString() != values::creature::type::player) {
			const sf::IntRect& sprite_rect = creatures_sprites[name][0].getTextureRect();

			creature_info inf{};
			inf.name = name;
			inf.size.x = static_cast<std::uint8_t>(sprite_rect.width);
			inf.size.y = static_cast<std::uint8_t>(sprite_rect.height);

			try_assign(current_creature, keys::creature::hp,            inf.base_hp,                  name);
			try_assign(current_creature, keys::creature::hp_pl,         inf.hp_per_level,             name);
			try_assign(current_creature, keys::creature::phys_power,    inf.base_physical_power,      name);
			try_assign(current_creature, keys::creature::phys_power_pl, inf.physical_power_per_level, name);
			try_assign(current_creature, keys::creature::armor,         inf.base_armor,               name);
			try_assign(current_creature, keys::creature::armor_pl,      inf.armor_per_level,          name);
			try_assign(current_creature, keys::creature::resist,        inf.base_resist,              name);
			try_assign(current_creature, keys::creature::resist_pl,     inf.resist_per_level,         name);
			try_assign(current_creature, keys::creature::crit,          inf.base_crit_chance,         name);
			try_assign(current_creature, keys::creature::crit_pl,       inf.crit_chance_per_level,    name);
			try_assign(current_creature, keys::creature::move_speed,    inf.base_move_speed,          name);
			try_assign(current_creature, keys::creature::move_speed_pl, inf.move_speed_per_level,     name);

			std::string type;
			try_assign(current_creature, keys::creature::type, type, name);
			if (type == values::creature::type::creep) {
				creature_info::creep_stats cs{};
				try_assign(current_creature, keys::creature::spawner::burst_duration, cs.spawn_count, name);
				try_assign(current_creature, keys::creature::spawner::burst_interval, cs.spawn_interval, name);
				try_assign(current_creature, keys::creature::spawner::burst_inner_interval, cs.spawn_long_interval, name);
				inf.other = cs;

			} else if (type == values::creature::type::creep_boss) {
				// nothing yet
				creature_info::creep_boss_stats cbs{};
				inf.other = cbs;

			} else if (type == values::creature::type::player) {
				creature_info::player_stats ps{};
				try_assign(current_creature, keys::player::mag_power_pl, ps.magic_power_per_level, name);
				try_assign(current_creature, keys::player::mag_power, ps.magic_power, name);
				try_assign(current_creature, keys::player::mana, ps.mana, name);
				try_assign(current_creature, keys::player::mana_pl, ps.mana_per_level, name);
				inf.other = ps;

			} else {
				spdlog::error("Key {} in {}: invalid value.", keys::creature::type, name);
				spdlog::error("Acceptable values are: {}, {}, {}", values::creature::type::player, values::creature::type::creep, values::creature::type::creep_boss);
			}

			creatures.emplace(name, inf);

			Json::Value& maps_ = current_creature[keys::creature::map::list];
			std::vector<std::string> map_names = maps_.getMemberNames();

			if (std::holds_alternative<creature_info::player_stats>(inf.other) && !map_names.empty()) {
				spdlog::warn("Player class {} has been marked with map availability. Ignoring.", name);
				continue;
			}

			mob_map_rinfo map_rinfo;
			for (const std::string& map : map_names) {
				auto map_inf_it = maps.find(map);
				if (map_inf_it == maps.end()) {
					spdlog::warn("Creature {} is marked available for map {}, but map {} does not exist.", name, map, map);
					continue;
				}
				spdlog::debug("Adding {} to creature set of map {}.", name, map);

				Json::Value& current_map = maps_[map];

				std::string source{};
				source.reserve(name.size() + 1 + map.size());
				source.append(name).append(":").append(map);

				if (current_map.isMember(keys::creature::map::min_level)) {
					try_assign(current_map, keys::creature::map::min_level, map_rinfo.min_level, map);
				} else {
					map_rinfo.min_level = 0u;
					spdlog::debug("{} for creature {} and map {} field is missing, setting a value of {}."
							, keys::creature::map::min_level, name, map, map_rinfo.min_level);
				}

				if (current_map.isMember(keys::creature::map::max_level)) {
					try_assign(current_map, keys::creature::map::max_level, map_rinfo.max_level, map);
				} else {
					map_rinfo.max_level = std::numeric_limits<unsigned short>::max();
					spdlog::debug("{} for creature {} and map {} field is missing, setting a value of {}."
							, keys::creature::map::max_level, name, map, map_rinfo.max_level);
				}

				if (current_map.isMember(keys::creature::map::populate_factor)) {
					try_assign(current_map, keys::creature::map::populate_factor, map_rinfo.populate_factor, map);
				} else {
					map_rinfo.populate_factor = 1;
					spdlog::debug("{} for creature {} and map {} field is missing, setting a value of {}."
							, keys::creature::map::populate_factor, name, map, map_rinfo.populate_factor);
				}
				creatures_info_per_map[map_inf_it->first][name] = map_rinfo;
			}
		}
	}
}

void resources::parse_maps() noexcept {
	spdlog::debug("Parsing maps infos");
	namespace tags = keys::map;

	auto map_list = maps_json.getMemberNames();
	for (const std::string& map_name : map_list) {
		const Json::Value& map = maps_json[map_name.data()];

		map_info current_map{};

		try_assign(map, tags::sizes::width, current_map.size.width, map_name);
		try_assign(map, tags::sizes::height, current_map.size.height, map_name);

		unsigned int i = 0;
		for (const Json::Value& zone : map[tags::categories::zones]) {
			++i;
			std::string source = map_name + ", in zone " + std::to_string(i) + " (counting from 1)";
			const Json::Value& rooms_properties = try_get(zone, tags::categories::rooms, source);
			const Json::Value& holes_properties = try_get(zone, tags::categories::holes, source);
			current_map.rooms_props.push_back(
					{
							{
									try_get_as<float>(rooms_properties, tags::zones::avg_size, source),
									try_get_as<float>(rooms_properties, tags::zones::size_deviation, source),
									try_get_as<float>(rooms_properties, tags::zones::borders_fuzzinness, source),
									try_get_as<float>(rooms_properties, tags::zones::borders_fuzzy_deviation, source),
									try_get_as<unsigned int>(rooms_properties, tags::zones::borders_fuzzy_distance, source),
									try_get_as<unsigned int>(rooms_properties, tags::zones::min_height, source),
									try_get_as<unsigned int>(rooms_properties, tags::zones::max_height, source)
							},
							{
									try_get_as<float>(holes_properties, tags::zones::avg_size, source),
									try_get_as<float>(holes_properties, tags::zones::size_deviation, source),
									try_get_as<float>(holes_properties, tags::zones::borders_fuzzinness, source),
									try_get_as<float>(holes_properties, tags::zones::borders_fuzzy_deviation, source),
									try_get_as<unsigned>(holes_properties, tags::zones::borders_fuzzy_distance, source),
									try_get_as<unsigned>(holes_properties, tags::zones::min_height, source),
									try_get_as<unsigned>(holes_properties, tags::zones::max_height, source)
							},
							try_get_as<float>(zone, tags::zones::avg_rooms_n, source),
							try_get_as<float>(zone, tags::zones::rooms_n_dev, source),
							try_get_as<float>(zone, tags::zones::avg_holes_n, source),
							try_get_as<float>(zone, tags::zones::holes_n_dev, source)
					}
			);
		}

		const Json::Value& halls_properties = try_get(map, tags::categories::halls, map_name);
		try_assign(halls_properties, tags::halls::curly_segment_avg_size, current_map.hallways_props.curly_segment_avg_size, map_name);
		try_assign(halls_properties, tags::halls::curly_segment_size_dev, current_map.hallways_props.curly_segment_size_dev, map_name);
		try_assign(halls_properties, tags::halls::curly_min_distance, current_map.hallways_props.curly_min_distance, map_name);
		try_assign(halls_properties, tags::halls::curliness, current_map.hallways_props.curliness, map_name);
		try_assign(halls_properties, tags::halls::avg_width, current_map.hallways_props.avg_width, map_name);
		try_assign(halls_properties, tags::halls::width_dev, current_map.hallways_props.width_dev, map_name);
		try_assign(halls_properties, tags::halls::min_width, current_map.hallways_props.min_width, map_name);
		try_assign(halls_properties, tags::halls::max_width, current_map.hallways_props.max_width, map_name);

		try_assign(map, tags::chests::rubbish_min, current_map.rubbish_chest.min, map_name);
		try_assign(map, tags::chests::rubbish_max, current_map.rubbish_chest.max, map_name);
		try_assign(map, tags::chests::wooden_min , current_map.wooden_chest.min , map_name);
		try_assign(map, tags::chests::wooden_max , current_map.wooden_chest.max , map_name);
		try_assign(map, tags::chests::magic_min  , current_map.magic_chest.min  , map_name);
		try_assign(map, tags::chests::magic_max  , current_map.magic_chest.max  , map_name);
		try_assign(map, tags::chests::iron_min   , current_map.iron_chest.min   , map_name);
		try_assign(map, tags::chests::iron_max   , current_map.iron_chest.max   , map_name);

		maps.emplace(map_name, current_map);
	}

}

void resources::load_creature_sprites(const std::string& name, const Json::Value& values) noexcept {
	spdlog::trace("> loading creature sprite: {}", name);

	sf::Texture& the_texture = texture;

	auto load_part = [&the_texture](const Json::Value& sub_value, std::string_view name_) -> sf::Sprite {
		sf::Sprite spr;
		spr.setTexture(the_texture);
		spr.setTextureRect(sf::IntRect(
				try_get_as<int>(sub_value, keys::sprites::geometry::x, name_)
				, try_get_as<int>(sub_value, keys::sprites::geometry::y, name_)
				, try_get_as<int>(sub_value, keys::sprites::geometry::width, name_)
				, try_get_as<int>(sub_value, keys::sprites::geometry::height, name_))
		);
		return spr;
	};

	auto get_or_flip = [&values, &load_part](std::string_view name_, const sf::Sprite& spr, float xscale, float yscale) -> sf::Sprite {
		sf::Sprite ans;
		if (values.isMember(name_.data())) {
			ans = load_part(values[name_.data()], name_);
		} else {
			ans = spr;
			ans.scale(xscale, yscale);
		}
		return ans;
	};

	sf::Sprite top       = load_part(try_get(values, keys::sprites::orientation::top, name), name + ": " + keys::sprites::orientation::top);
	sf::Sprite top_right = load_part(try_get(values, keys::sprites::orientation::top_right, name), name + ": " + keys::sprites::orientation::top_right);
	sf::Sprite right     = load_part(try_get(values, keys::sprites::orientation::right, name), name + ": " + keys::sprites::orientation::right);

	sf::Sprite bot_right = get_or_flip(keys::sprites::orientation::bot_right, top_right,  1.f, -1.f); // flip around vertical axis
	sf::Sprite bot       = get_or_flip(keys::sprites::orientation::bot      , top      ,  1.f, -1.f);
	sf::Sprite bot_left  = get_or_flip(keys::sprites::orientation::bot_left , bot_right, -1.f,  1.f); // flip around hztal axis
	sf::Sprite left      = get_or_flip(keys::sprites::orientation::left     , right    , -1.f,  1.f);
	sf::Sprite top_left  = get_or_flip(keys::sprites::orientation::top_left , top_right, -1.f,  1.f);

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

	auto load_part = [&the_texture](const Json::Value& sub_value, std::string_view name_) -> sf::Sprite {
		sf::Sprite spr;
		spr.setTexture(the_texture);
		spr.setTextureRect(sf::IntRect(
				try_get_as<int>(sub_value, keys::sprites::geometry::x, name_)
				, try_get_as<int>(sub_value, keys::sprites::geometry::y, name_)
				, try_get_as<int>(sub_value, keys::sprites::geometry::width, name_)
				, try_get_as<int>(sub_value, keys::sprites::geometry::height, name_))
		);
		return spr;
	};

	auto& array = maps_sprites.emplace(std::make_pair(name, std::array<sf::Sprite, tiles_count>())).first->second;
	array[static_cast<unsigned>(tiles::hole)       ] = load_part(try_get(values, keys::sprites::tiles::hole, name), name + ": " + keys::sprites::tiles::hole);
	array[static_cast<unsigned>(tiles::walkable)   ] = load_part(try_get(values, keys::sprites::tiles::walkable, name), name + ": " + keys::sprites::tiles::walkable);
	array[static_cast<unsigned>(tiles::empty_space)] = load_part(try_get(values, keys::sprites::tiles::empty_space, name), name + ": " + keys::sprites::tiles::empty_space);
	array[static_cast<unsigned>(tiles::wall)       ] = load_part(try_get(values, keys::sprites::tiles::wall, name), name + ": " + keys::sprites::tiles::wall);

}

std::vector<std::pair<std::string_view, resources::mob_map_rinfo>>
resources::get_creatures_for_level(unsigned int level, std::string_view map_name) const noexcept {

	std::vector<std::pair<std::string_view, resources::mob_map_rinfo>> ans;
	try {
		for (const auto& pair : creatures_info_per_map.at(map_name)) {
			const mob_map_rinfo& current = pair.second;
			if (current.max_level <= level && current.min_level >= level) {
				ans.emplace_back(pair);
			}
		}
	} catch (const std::out_of_range&) {
		spdlog::error("Tried to access map {}, which is not in the map dataset. [internal error]", map_name);
	}
	return ans;
}

#pragma clang diagnostic pop
#pragma GCC diagnostic pop
