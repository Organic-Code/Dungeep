#ifndef DUNGEEP_MAP_HPP
#define DUNGEEP_MAP_HPP

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

#include <iosfwd>
#include <vector>
#include <random>
#include <chrono>
#include "tiles.hpp"
#include "geometry.hpp"

struct zone_gen_properties {
	/** All parameters >= 0
	 *
	 */

	float avg_size;
	float size_deviation;
	float borders_fuzzinness;
	float borders_fuzzy_deviation;
	unsigned int borders_fuzzy_distance;
	unsigned int min_height;
	unsigned int max_height;
};

struct room_gen_properties {
	zone_gen_properties rooms_properties;
	zone_gen_properties holes_properties;

	float avg_rooms_n;
	float rooms_n_dev;
	float avg_holes_n;
	float holes_n_dev;
};

struct hallway_gen_properties {
	float curliness;
	float curly_min_distance;
	float curly_segment_avg_size;
	float curly_segment_size_dev;

	float avg_width;
	float width_dev;
	unsigned int min_width;
	unsigned int max_width;
};

class map {

public:

	struct size_type {
		unsigned int width, height;
	};

	map() = default;

	void generate(size_type size
			, const std::vector<room_gen_properties>& rooms_properties
			, const hallway_gen_properties& hallway_properties);

	std::vector<tiles>& operator[](std::vector<tiles>::size_type sz) {
		return m_tiles[sz];
	}

	const std::vector<tiles>& operator[](std::vector<tiles>::size_type sz) const {
		return m_tiles[sz];
	}

	size_type size() const noexcept {
		return {static_cast<unsigned int>(m_tiles.size()),
		        static_cast<unsigned int>(m_tiles.front().size())};
	}

	std::vector<dungeep::direction> path_to(const dungeep::point_i& source, const dungeep::point_i& destination
			, float wall_crossing_penalty = 30.f, int max_depth = std::numeric_limits<int>::max()) const;

	std::vector<dungeep::point_i> path_to_pt(const dungeep::point_i& source, const dungeep::point_i& destination, float wall_crossing_penalty = 30.f) const;



private:

	struct map_area {
		unsigned int x, y;
		unsigned int width, height;
	};

	static void add_fuzziness(std::vector<std::vector<tiles>>& generated_room, const zone_gen_properties& rp
			, const map_area& tiles_area, tiles tile, std::normal_distribution<float>& zone_fuzziness);

	static float gen_positive(float avg, float dev);

	void ensure_pathing(const std::vector<dungeep::point_ui>& rooms_center, const hallway_gen_properties&);

	void ensure_tworoom_path(const dungeep::point_ui& r1_center, const dungeep::point_ui& r2_center, const hallway_gen_properties&);

	dungeep::point_ui generate_holed_room(const room_gen_properties& rp, unsigned int hole_count);

	static dungeep::point_ui generate_zone_dimensions(const zone_gen_properties& zgp);


	void generate_tiles(const zone_gen_properties& rp, map_area tiles_area, tiles tile);

	dungeep::point_ui
	find_zone_filled_with(dungeep::point_ui zone_dim, tiles tile)
	const noexcept {
		return find_zone_filled_with(zone_dim, tile, {0,0, size().width - 1, size().height - 1});
	}

	dungeep::point_ui find_zone_filled_with(dungeep::point_ui zone_dim, tiles tile, map_area sub_area) const noexcept;

	std::vector<std::vector<tiles>> m_tiles{};

public:
	// debug infos:
	std::chrono::milliseconds total_generation_time{};
	std::chrono::milliseconds fuzzy_generation_time{};
	std::chrono::milliseconds rooms_generation_time{};
	std::chrono::milliseconds halls_generation_time{};

	unsigned int expected_room_count{};
	unsigned int expected_hole_count{};
	unsigned int actual_room_count{};
	unsigned int actual_hole_count{};
};


#endif //DUNGEEP_MAP_GENERATOR_HPP
