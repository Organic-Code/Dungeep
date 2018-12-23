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

#include <string_view>
#include <fstream>
#include <algorithm>

#include "resource_manager.hpp"


namespace {
	constexpr bool disable_resources = false; // everything might not work if disabled.
}

namespace paths {
	namespace {
#define RESOURCE_FOLDER "resources/"
		constexpr std::string_view resource_folder = RESOURCE_FOLDER;
		constexpr std::string_view map_file = RESOURCE_FOLDER"maps.json";
#undef RESOURCE_FOLDER
	}
}

namespace tags {
	namespace categories {
		namespace {
			constexpr const char* zones = "zones properties";
			constexpr const char* rooms = "rooms properties";
			constexpr const char* holes = "holes properties";
			constexpr const char* halls = "halls properties";
		}
	}
	namespace sizes {
		namespace {
			constexpr const char * width = "width";
			constexpr const char * height = "height";
		}
	}
	namespace zones {
		namespace {
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
	}
	namespace halls {
		namespace {
			constexpr const char* curliness = "curliness";
			constexpr const char* curly_min_distance = "curly minimum distance";
			constexpr const char* curly_segment_avg_size = "curly segment average size";
			constexpr const char* curly_segment_size_dev = "curly segment size deviation";
			constexpr const char* avg_width = "average width";
			constexpr const char* width_dev = "width deviation";
			constexpr const char* min_width = "minimum width";
			constexpr const char* max_width = "maximum width";
		}
	}
}

resource_manager_t resource_manager{};

resource_manager_t::resource_manager_t() noexcept : maps{} {
	if constexpr (disable_resources) {
		return;
	}
	std::ifstream maps_file(paths::map_file.data(), std::ios_base::in);
	if (!maps_file) {
		throw std::runtime_error("Failed to load resource file " + std::string(paths::map_file));
	}
	maps_file >> maps;
	map_list = maps.getMemberNames();
}


const std::vector<std::string>& resource_manager_t::get_map_list() const {
	return map_list;
}

std::tuple<map::size_type, std::vector<room_gen_properties>, hallway_gen_properties>
resource_manager_t::get_map(std::string_view map_name) const {
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

void resource_manager_t::save_map(std::string_view map_name, map::size_type size
		, const std::vector<room_gen_properties>& room_properties, const hallway_gen_properties& halls_properties) {
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
