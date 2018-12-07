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
///  		warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or       ///
///  copyright holders X be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,      ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <map.hpp>
#include <random>
#include <algorithm>
#include <iostream>

#include "map.hpp"

map::map(size_type size, const std::vector<room_gen_properties>& rooms_properties, std::mt19937_64& random_engine)
	: m_tiles{size.width, std::vector<tiles>{size.height, tiles::empty_space}}
{
	assert(size.width > 0);
	assert(!rooms_properties.empty());

	std::vector<dungeep::uis_point> rooms_center;
	rooms_center.reserve(static_cast<unsigned long>(rooms_properties.size() * rooms_properties[0].avg_rooms_n));

	for (const room_gen_properties& rp : rooms_properties) {
		auto rooms_n = gen_positive(rp.avg_rooms_n, rp.rooms_n_dev, random_engine);
		auto holes_n = gen_positive(rp.avg_holes_n, rp.holes_n_dev, random_engine);

		for (auto i = 0u ; i < rooms_n - 1; ++i) {

			unsigned int hfr = std::min(
					static_cast<unsigned int>(holes_n),
					static_cast<unsigned int>(gen_positive(holes_n / (rooms_n - i), 2.5f, random_engine))
			);

			holes_n -= hfr;
			dungeep::uis_point room = generate_holed_room(rp, hfr, random_engine);
			if (room.x != 0 && room.y != 0) {
				rooms_center.push_back(room);
			}
		}
		dungeep::uis_point room = generate_holed_room(rp, static_cast<unsigned int>(holes_n), random_engine);
		if (room.x != 0 && room.y != 0) {
			rooms_center.push_back(room);
		}
	}

	ensure_pathing(rooms_center);
}

void map::ensure_pathing(const std::vector<dungeep::uis_point>& rooms_center) {
	using dungeep::uis_point;
	for (const uis_point& room : rooms_center) {
		ensure_oneroom_path(room);
	}

	for (const uis_point& room_1 : rooms_center) {
		for (const uis_point& room_2 : rooms_center) {
			if (&room_1 == &room_2) {
				continue;
			}
			ensure_tworoom_path(room_1, room_2);
		}
	}
}

void map::ensure_oneroom_path(const dungeep::uis_point& room_center) {

}

void map::ensure_tworoom_path(const dungeep::uis_point& r1_center, const dungeep::uis_point& r2_center) {

}

dungeep::uis_point map::generate_holed_room(const room_gen_properties& rp, unsigned int hole_count, std::mt19937_64& random_engine) {

	bool failed;
	int fail_count = 0;

	dungeep::uis_point room_dim{}, room_pos{};

	do {
		failed = false;
		room_dim = generate_zone_dimensions(rp.rooms_properties, random_engine);
		room_pos = find_zone_filled_with(room_dim, tiles::empty_space, random_engine);
		if (room_pos.x == 0 && room_pos.y == 0) {
			failed = true;
			++fail_count;
		}
	} while (failed && fail_count < 30);
	if (failed) {
		return {0, 0};
	}

	map_area room_area{room_pos.x, room_pos.y, room_dim.x, room_dim.y};
	generate_tiles(rp.rooms_properties, room_area, tiles::walkable, random_engine);

	for (auto i = 0u ; i < hole_count ; ++i) {
		fail_count = 0;
		do {
			dungeep::uis_point hole_dim = generate_zone_dimensions(rp.holes_properties, random_engine);
			dungeep::uis_point hole_pos = find_zone_filled_with(hole_dim, tiles::walkable, random_engine, room_area);
			if (hole_pos.x == 0 && hole_pos.y == 0) {
				++fail_count;
				failed = true;
			} else {
				generate_tiles(rp.holes_properties, {hole_pos.x, hole_pos.y, hole_dim.x, hole_dim.y}, tiles::hole, random_engine);
				failed = false;
			}
		} while (failed && fail_count < 10);
	}

	return room_pos;
}

void map::generate_tiles(const zone_gen_properties& rp, map_area tiles_area, tiles tile, std::mt19937_64& random_engine) {
	std::normal_distribution zone_fuzziness(0.f, rp.borders_fuzzy_deviation);

	auto array_shift = static_cast<unsigned int>(rp.borders_fuzzinness + rp.borders_fuzzy_deviation * 4) * 2;
	std::vector<std::vector<tiles>> generated_room{
		tiles_area.width + array_shift,
		std::vector<tiles>{
			tiles_area.height + array_shift,
			tiles::none
		}
	};

	for (auto i = 0 ; i < tiles_area.width ; ++i) {
		for (auto j = 0 ; j < tiles_area.height ; ++j) {
			generated_room[i + array_shift][j + array_shift] = tile;
		}
	}

	map_area ar{};
	ar.x = ar.y = array_shift;
	ar.width = tiles_area.width;
	ar.height = tiles_area.height;
	add_fuzziness(generated_room, rp, ar, tile, zone_fuzziness, random_engine);

	for (auto i = std::max<int>(array_shift - tiles_area.x, 0) ; i < generated_room.size() && tiles_area.x + i < m_tiles.size() + array_shift ; ++i) {
		for (auto j = std::max<int>(array_shift - tiles_area.y, 0) ; j < generated_room[i].size() && tiles_area.y + j < m_tiles[i].size() + array_shift; ++j) {
			if (generated_room[i][j] != tiles::none) {
				m_tiles[i + tiles_area.x - array_shift][j + tiles_area.y - array_shift] = generated_room[i][j];
			}
		}
	}
}

void map::add_fuzziness(std::vector<std::vector<tiles>>& generated_room, const zone_gen_properties& rp, const map_area& tiles_area,
                        tiles tile, std::normal_distribution<float>& zone_fuzziness, std::mt19937_64& random_engine) {
	float current_delta = 0.f;
	float delta_step = 0.f;

	auto generate_fuzziness = [&](unsigned int min, unsigned int max, float delta_max, auto&& assigner) {

		for (auto i = min; i < max ; ++i) {
			if ((i % rp.borders_fuzzy_distance) == (min % rp.borders_fuzzy_distance)) {
				float target_delta = std::clamp(zone_fuzziness(random_engine) * rp.borders_fuzzinness, -delta_max, delta_max);
				delta_step = (target_delta - current_delta) / static_cast<float>(rp.borders_fuzzy_distance);
			}
			current_delta += delta_step;

			if (current_delta < 0) {
				for (auto j = static_cast<int>(current_delta - 0.9f) ; j < 0 ; ++j) {
					assigner(i, j, true);
				}
			} else {
				for (auto j = 0 ; j < current_delta + 0.9f; ++j) {
					assigner(i, j, false);
				}
			}
		}
	};

	// fuzziness on y axis (left)
	generate_fuzziness(tiles_area.y, tiles_area.y + tiles_area.height, tiles_area.height / 2.f, [&](unsigned int i, int j, bool fill) {
		int idx = j + tiles_area.x;
		if (idx > 0 && idx < generated_room.size()) {
			generated_room[idx][i] = fill ? tile : tiles::none;
		}
	});

	// fuzziness on y axis (right)
	generate_fuzziness(tiles_area.y, tiles_area.y + tiles_area.height, tiles_area.height / 2.f, [&](unsigned int i, int j, bool fill) {
		int idx = j + tiles_area.x + tiles_area.width;
		if (idx > 0 && idx < generated_room.size()) {
			generated_room[idx][i] = fill ? tiles::none : tile;
		}
	});

	// fuzziness on x axis (top)
	generate_fuzziness(tiles_area.x, tiles_area.x + tiles_area.width, tiles_area.width / 2.f, [&](unsigned int i, int j, bool fill) {
		int idx = j + tiles_area.y;
		if (idx > 0 && idx < generated_room[i].size()) {
			generated_room[i][idx] = fill ? tile : tiles::none;
		}
	});

	// fuzziness on x axis (bottom)
	generate_fuzziness(tiles_area.x, tiles_area.x + tiles_area.width, tiles_area.width / 2.f, [&](unsigned int i, int j, bool fill) {
		int idx = j + tiles_area.y + tiles_area.height;
		if (idx > 0 && idx < generated_room[i].size()) {
			generated_room[i][idx] = fill ? tiles::none : tile;
		}
	});

}

dungeep::uis_point
map::find_zone_filled_with(dungeep::uis_point zone_dim, tiles tile, std::mt19937_64& random_engine, map_area area) const noexcept {
	assert(zone_dim.x < area.width);
	assert(zone_dim.y < area.height);
	assert(area.x + area.width  < size().width);
	assert(area.y + area.height < size().height);

	std::uniform_int_distribution<unsigned int> uid_x(area.x, area.x + area.width);
	std::uniform_int_distribution<unsigned int> uid_y(area.y, area.y + area.height);

	auto is_good_spot = [this, &tile, &zone_dim, &area](dungeep::uis_point spot){
		if (spot.x < area.x || spot.y < area.y) {
			return false;
		}
		if (spot.x + zone_dim.x > area.x + area.width || spot.y + zone_dim.y > area.y + area.height) {
			return false;
		}

		for (auto i = 3u ; i + 3 < zone_dim.x; ++i) {
			for (auto j = 3u ; j + 3 < zone_dim.y; ++j) {
				if (m_tiles[i + spot.x][j + spot.y] != tile) {
					return false;
				}
			}
		}
		return true;
	};

	dungeep::uis_point ans{};

	int fail_count = 0;
	do {
		ans.x = uid_x(random_engine);
		ans.y = uid_y(random_engine);
	} while(!is_good_spot(ans) && fail_count++ < 100);

	if (fail_count >= 100) {
		ans = {0u, 0u};
	}

	return ans;
}

float map::gen_positive(float avg, float dev, std::mt19937_64& engine) {
	std::normal_distribution dist{avg, dev};
	float nbr = dist(engine);
	return nbr > 0 ? nbr : 0.f;
}

dungeep::uis_point map::generate_zone_dimensions(const zone_gen_properties& zgp, std::mt19937_64& random_engine) {

	float min_size = zgp.min_height;
	min_size *= min_size;
	float max_size = zgp.max_height;
	max_size *= max_size;

	float room_size = std::clamp(gen_positive(zgp.avg_size, zgp.size_deviation, random_engine), min_size, max_size);

	unsigned int avg_room_dim = (zgp.max_height + zgp.min_height) / 2;
	dungeep::uis_point room_dim{};
	room_dim.x = std::clamp(static_cast<unsigned int>(gen_positive(avg_room_dim, 2.f * avg_room_dim / 5.f, random_engine)), zgp.min_height, zgp.max_height);
	room_dim.y = static_cast<unsigned int>(room_size / room_dim.x);

	return room_dim;
}

std::ostream& operator<<(std::ostream& o, const map& m) {
	auto size = m.size();
	for (auto i = 0u ; i < size.height ; ++i) {
		for (auto j = 0u ; j < size.width ; ++j) {
			switch (m[j][i]) {
				case tiles::wall:
					o << '#';
					break;
				case tiles::empty_space:
					o << '\'';
					break;
				case tiles::hole:
					o << 'O';
					break;
				case tiles::walkable:
					o << ' ';
					break;
				case tiles::none:
					o << '*';
					break;
			}
		}
		o << '\n';
	}
	return o;
}
