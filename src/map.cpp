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

#include <random>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <set>
#include <map.hpp>
#include <chrono>
#include <quadtree.hpp>


#include "map.hpp"

void map::generate(size_type size, const std::vector<room_gen_properties>& rooms_properties, const hallway_gen_properties& hgp, unsigned long seed) {
	using std::chrono::duration_cast;
	using std::chrono::system_clock;
	using std::chrono::milliseconds;

	std::mt19937_64 random_engine(seed);

	fuzzy_generation_time = milliseconds{0};
	expected_hole_count = expected_room_count = actual_hole_count = actual_room_count = 0;

	auto starting_tp = system_clock::now();

	m_tiles = {size.width, std::vector<tiles>{size.height, tiles::empty_space}};
	assert(size.width > 0);
	assert(!rooms_properties.empty());

	std::vector<dungeep::point_ui> rooms_center;
	rooms_center.reserve(rooms_properties.size() * static_cast<unsigned long>(rooms_properties[0].avg_rooms_n));

	auto room_starting_tp = system_clock::now();
	for (const room_gen_properties& rp : rooms_properties) {
		auto rooms_n = gen_positive(rp.avg_rooms_n, rp.rooms_n_dev, random_engine);
		auto holes_n = gen_positive(rp.avg_holes_n, rp.holes_n_dev, random_engine);

		expected_room_count += static_cast<unsigned>(rooms_n);
		expected_hole_count += static_cast<unsigned>(holes_n);

		for (auto i = 0 ; static_cast<float>(i) < (rooms_n - 0.3f) ; ++i) {

			unsigned int hfr = std::min(
					static_cast<unsigned int>(holes_n),
					static_cast<unsigned int>(gen_positive(holes_n / (rooms_n - static_cast<float>(i)), 2.5f, random_engine))
			);

			holes_n -= static_cast<float>(hfr);
			dungeep::point_ui room = generate_holed_room(rp, hfr, random_engine);
			if (room.x != 0 && room.y != 0) {
				++actual_room_count;
				rooms_center.push_back(room);
			}
		}
		dungeep::point_ui room = generate_holed_room(rp, static_cast<unsigned int>(holes_n), random_engine);
		if (room.x != 0 && room.y != 0) {
			++actual_room_count;
			rooms_center.push_back(room);
		}
	}
	rooms_generation_time = duration_cast<milliseconds>(system_clock::now() - room_starting_tp);

	auto halls_tp = system_clock::now();
	ensure_pathing(rooms_center, hgp, random_engine);
	halls_generation_time = duration_cast<milliseconds>(system_clock::now() - halls_tp);

	total_generation_time = duration_cast<milliseconds>(system_clock::now() - starting_tp);
}

void map::ensure_pathing(const std::vector<dungeep::point_ui>& rooms_center, const hallway_gen_properties& properties, std::mt19937_64& re) {

	struct collider {
		const dungeep::area_f& hitbox() const {
			return hitbox_;
		}

		dungeep::area_f hitbox_;
		dungeep::point_ui room_center;
	};
	dungeep::quadtree<collider> qt(
			dungeep::area_f{
					dungeep::point_f{0.f, 0.f},
					dungeep::point_f{static_cast<float>(size().width), static_cast<float>(size().height)}
			}
	);

	float avg_distance = 0.f;
	std::vector<float> distances;
	distances.reserve(rooms_center.size() * (rooms_center.size() - 1));
	for (const dungeep::point_ui& center_1 : rooms_center) {
		for (const dungeep::point_ui& center_2 : rooms_center) {
			float distance = (dungeep::point_i{center_1} - dungeep::point_i{center_2}).length<float>();
			avg_distance += distance;
			distances.push_back(distance);
		}

		auto x = static_cast<float>(center_1.x);
		auto y = static_cast<float>(center_1.y);
		dungeep::area_f htbox{
			dungeep::point_f{x - 1.f, y - 1.f},
			dungeep::point_f{x + 1.f, y + 1.f}
		};
		qt.insert({htbox, center_1});
	}

	avg_distance /= static_cast<float>(rooms_center.size() * rooms_center.size());
	std::sort(distances.begin(), distances.end());

	float selected_distance = std::max(distances[distances.size() / 8], avg_distance / 2.f);
	for (const dungeep::point_ui& room : rooms_center) {
		auto x = static_cast<float>(room.x);
		auto y = static_cast<float>(room.y);
		dungeep::area_f htbox{
				dungeep::point_f{x - selected_distance, y - selected_distance},
				dungeep::point_f{x + selected_distance, y + selected_distance}
		};

		qt.visit(htbox, [this, &properties, &room, &re](dungeep::quadtree<collider>::iterator it) {
			if (re() % 3) {
				if (path_to(dungeep::point_i(room), dungeep::point_i(it->room_center), std::numeric_limits<float>::infinity()).empty()) {
					ensure_tworoom_path(room, it->room_center, properties, re);
				}
			}
		});
	}

	for (auto i = 0u ; i + 1 < rooms_center.size() ; ++i) {
		if (path_to(dungeep::point_i(rooms_center[i]), dungeep::point_i(rooms_center[i + 1]), std::numeric_limits<float>::infinity()).empty()) {
			ensure_tworoom_path(rooms_center[i], rooms_center[i + 1], properties, re);
		}
	}

}

void map::ensure_tworoom_path(const dungeep::point_ui& r1_center, const dungeep::point_ui& r2_center, const hallway_gen_properties& properties, std::mt19937_64& re) {
	using dungeep::point_i;
	using dungeep::point_f;

	auto place_point = [this](const point_i& point, int width) {
		auto valid_x = [this](int idx) {
			return idx > 0 && idx < static_cast<int>(m_tiles.size());
		};
		auto valid_y = [this](int idx) {
			return idx > 0 && idx < static_cast<int>(m_tiles.front().size());
		};
		if (valid_x(point.x) && valid_y(point.y)) {
			m_tiles[static_cast<unsigned>(point.x)][static_cast<unsigned>(point.y)] = tiles::walkable;
		}
		for (int i = width / -2 ; i < (width + 1) / 2 ; ++i) {
			if (valid_y(point.y)) {
				if (valid_x(point.x + i)) {
					m_tiles[static_cast<unsigned>(point.x + i)][static_cast<unsigned>(point.y)] = tiles::walkable;
				}
				if (valid_x(point.x - i)) {
					m_tiles[static_cast<unsigned>(point.x - i)][static_cast<unsigned>(point.y)] = tiles::walkable;
				}
			}
			if (valid_x(point.x)) {
				if (valid_y(point.y + i)) {
					m_tiles[static_cast<unsigned>(point.x)][static_cast<unsigned>(point.y + i)] = tiles::walkable;
				}
				if (valid_y(point.y - i)) {
					m_tiles[static_cast<unsigned>(point.x)][static_cast<unsigned>(point.y - i)] = tiles::walkable;
				}
			}
		}
	};

	point_i dep{r1_center};
	point_i arr{r2_center};

	std::vector<dungeep::point_i> stop_offs;
	stop_offs.push_back(dep);

	if ((dep - arr).length<float>() >= properties.curly_min_distance) {
		std::normal_distribution curliness(0.f, std::max(properties.curliness, 0.001f));
		std::normal_distribution curly_size(properties.curly_segment_avg_size, std::max(properties.curly_segment_size_dev, 0.001f));
		do {
			point_i translate = arr - stop_offs.back();
			float segment_size = std::max(curly_size(re), properties.curly_min_distance);
			float segment_angle = curliness(re) * static_cast<float>(M_PI) / 4;

			translate.rotate(segment_angle);
			translate.scale_to(segment_size);

			stop_offs.push_back(translate + stop_offs.back());

		} while ((stop_offs.back() - arr).length<float>() >= properties.curly_min_distance);
	}

	stop_offs.push_back(arr);

	std::normal_distribution width(properties.avg_width, std::max(properties.width_dev, 0.001f));
	auto w = static_cast<int>(std::clamp(static_cast<unsigned int>(width(re)), properties.min_width, properties.max_width));
	for (auto i = 0u ; i + 1 < stop_offs.size() ; ++i) {
		point_i start = stop_offs[i];
		point_i end = stop_offs[i + 1];
		point_f translate(end - start);
		auto segment_length = translate.length();
		translate.scale_to(1.f);
		for (auto j = 0u ; j < static_cast<unsigned>(segment_length + 0.9f) ; ++j) {
			point_i current = point_i(point_f(stop_offs[i]) + static_cast<float>(j) * translate);
			place_point(current, w);
		}
	}
}

dungeep::point_ui map::generate_holed_room(const room_gen_properties& rp, unsigned int hole_count, std::mt19937_64& random_engine) {

	bool failed;
	int fail_count = 0;

	dungeep::point_ui room_dim{}, room_pos{};

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
			dungeep::point_ui hole_dim = generate_zone_dimensions(rp.holes_properties, random_engine);
			dungeep::point_ui hole_pos = find_zone_filled_with(hole_dim, tiles::walkable, random_engine, room_area);
			if (hole_pos.x == 0 && hole_pos.y == 0) {
				++fail_count;
				failed = true;
			} else {
				generate_tiles(rp.holes_properties, {hole_pos.x, hole_pos.y, hole_dim.x, hole_dim.y}, tiles::hole, random_engine);
				++actual_hole_count;
				failed = false;
			}
		} while (failed && fail_count < 10);
	}

	room_pos.x += room_dim.x / 2;
	room_pos.y += room_dim.y / 2;
	return room_pos;
}

void map::generate_tiles(const zone_gen_properties& rp, map_area tiles_area, tiles tile, std::mt19937_64& random_engine) {
	std::normal_distribution zone_fuzziness(0.f, std::max(rp.borders_fuzzy_deviation, 0.001f));

	auto array_shift = static_cast<unsigned int>(rp.borders_fuzzinness + rp.borders_fuzzy_deviation * 4) * 2;
	std::vector<std::vector<tiles>> generated_room{
		tiles_area.width + array_shift,
		std::vector<tiles>{
			tiles_area.height + array_shift,
			tiles::none
		}
	};

	for (auto i = 0u ; i < tiles_area.width ; ++i) {
		for (auto j = 0u ; j < tiles_area.height ; ++j) {
			generated_room[i + array_shift][j + array_shift] = tile;
		}
	}

	map_area ar{};
	ar.x = ar.y = array_shift;
	ar.width = tiles_area.width;
	ar.height = tiles_area.height;

	auto tp = std::chrono::system_clock::now();
	add_fuzziness(generated_room, rp, ar, tile, zone_fuzziness, random_engine);
	fuzzy_generation_time += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tp);

	unsigned int min_i = array_shift > tiles_area.x ? array_shift - tiles_area.x : 0u;
	unsigned int min_j = array_shift > tiles_area.y ? array_shift - tiles_area.y : 0u;

	for (auto i = min_i ; i < generated_room.size() && tiles_area.x + i < m_tiles.size() + array_shift ; ++i) {
		for (auto j = min_j ; j < generated_room[i].size() && tiles_area.y + j < m_tiles[i].size() + array_shift; ++j) {
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
				for (auto j = 0 ; j < static_cast<int>(current_delta + 0.9f); ++j) {
					assigner(i, j, false);
				}
			}
		}
	};

	// fuzziness on y axis (left)
	generate_fuzziness(tiles_area.y, tiles_area.y + tiles_area.height, static_cast<float>(tiles_area.height) / 2.f, [&](unsigned int i, int j, bool fill) {
		unsigned int idx = static_cast<unsigned>(j) + tiles_area.x;
		if (idx < generated_room.size()) {
			generated_room[idx][i] = fill ? tile : tiles::none;
		}
	});

	// fuzziness on y axis (right)
	generate_fuzziness(tiles_area.y, tiles_area.y + tiles_area.height, static_cast<float>(tiles_area.height) / 2.f, [&](unsigned int i, int j, bool fill) {
		unsigned int idx = static_cast<unsigned>(j) + tiles_area.x + tiles_area.width;
		if (idx < generated_room.size()) {
			generated_room[idx][i] = fill ? tiles::none : tile;
		}
	});

	// fuzziness on x axis (top)
	generate_fuzziness(tiles_area.x, tiles_area.x + tiles_area.width, static_cast<float>(tiles_area.width) / 2.f, [&](unsigned int i, int j, bool fill) {
		unsigned int idx = static_cast<unsigned>(j) + tiles_area.y;
		if (idx < generated_room[i].size()) {
			generated_room[i][idx] = fill ? tile : tiles::none;
		}
	});

	// fuzziness on x axis (bottom)
	generate_fuzziness(tiles_area.x, tiles_area.x + tiles_area.width, static_cast<float>(tiles_area.width) / 2.f, [&](unsigned int i, int j, bool fill) {
		unsigned int idx = static_cast<unsigned>(j) + tiles_area.y + tiles_area.height;
		if (idx < generated_room[i].size()) {
			generated_room[i][idx] = fill ? tiles::none : tile;
		}
	});

}

dungeep::point_ui
map::find_zone_filled_with(dungeep::point_ui zone_dim, tiles tile, std::mt19937_64& random_engine, map_area area) const noexcept {
	assert(zone_dim.x < area.width);
	assert(zone_dim.y < area.height);
	assert(area.x + area.width  < size().width);
	assert(area.y + area.height < size().height);

	std::uniform_int_distribution<unsigned int> uid_x(area.x, area.x + area.width);
	std::uniform_int_distribution<unsigned int> uid_y(area.y, area.y + area.height);

	auto is_good_spot = [this, &tile, &zone_dim, &area](dungeep::point_ui spot){
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

	dungeep::point_ui ans{};

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
	std::normal_distribution dist{avg, std::max(dev, 0.001f)};
	float nbr = dist(engine);
	return nbr > 0 ? nbr : 0.f;
}

dungeep::point_ui map::generate_zone_dimensions(const zone_gen_properties& zgp, std::mt19937_64& random_engine) {

	auto min_size = zgp.min_height;
	min_size *= min_size;
	auto max_size = zgp.max_height;
	max_size *= max_size;

	float room_size = std::clamp(gen_positive(zgp.avg_size, zgp.size_deviation, random_engine), static_cast<float>(min_size), static_cast<float>(max_size));

	float avg_room_dim = static_cast<float>(zgp.max_height + zgp.min_height) / 2.f;
	dungeep::point_ui room_dim{};
	room_dim.x = std::clamp(static_cast<unsigned int>(gen_positive(avg_room_dim, 0.4f * avg_room_dim, random_engine)), zgp.min_height, zgp.max_height);
	room_dim.y = static_cast<unsigned int>(room_size / static_cast<float>(room_dim.x));

	return room_dim;
}

std::vector<dungeep::direction> map::path_to(const dungeep::point_i& source, const dungeep::point_i& destination, float wall_crossing_penalty) const {
	using dungeep::point_i;
	using dungeep::direction;

	std::vector<direction> ans;
	if (source == destination) {
		return ans;
	}

	struct node {
		point_i pos{};
		float dist{};
		float heur{};

		direction parent{direction::none};

		float cost() const {
			return dist + heur;
		}

		bool operator<(const node& n) const noexcept {
			return cost() < n.cost();
		}

		bool operator==(const node& n) const noexcept {
			return pos == n.pos;
		}
	};
	auto cost_of = [&destination](const point_i& p) {
		return std::hypot(static_cast<float>(destination.x - p.x), static_cast<float>(destination.y - p.y));
	};
	auto hash = [](const node& n) noexcept {
		return n.pos.x + n.pos.y;
	};
	auto make_node = [&cost_of](const point_i& p, float distance) {
		return node{p, distance, cost_of(p)};
	};
	auto make_child_node = [&cost_of, this, &wall_crossing_penalty](const node& parent, const std::pair<point_i, direction>& translation) {
		dungeep::point_i new_pos = parent.pos + translation.first;
		auto tile = m_tiles[std::clamp(new_pos.x, 0, static_cast<int>(m_tiles.size() - 1))][std::clamp(new_pos.y, 0, static_cast<int>(m_tiles[0].size() - 1))];
		float penalty = tile != tiles::walkable ? wall_crossing_penalty : 0.f;

		return node{new_pos, parent.dist + std::hypot(static_cast<float>(translation.first.x), static_cast<float>(translation.first.y)) + penalty
			  , cost_of(new_pos), translation.second};
	};

	std::unordered_set<node, decltype(hash)> closed_list{0, std::move(hash)};
	std::set<node> open_list{};

	open_list.emplace(make_node(source, 0.f));

	constexpr std::array<std::pair<point_i, direction>, 8> possible_children = {
			std::pair{point_i{-1, -1}, direction::top_left },
			std::pair{point_i{ 0, -1}, direction::top      },
			std::pair{point_i{ 1, -1}, direction::top_right},
			std::pair{point_i{-1,  0}, direction::left     },
			std::pair{point_i{ 1,  0}, direction::right    },
			std::pair{point_i{-1,  1}, direction::bot_left },
			std::pair{point_i{ 0,  1}, direction::bot      },
			std::pair{point_i{ 1,  1}, direction::bot_right}
	};

	std::optional<node> ending_node{};
	do {
		auto smallest = open_list.begin();
		node q = *smallest;
		open_list.erase(smallest);

		auto it_pair = closed_list.insert(q);
		if (!it_pair.second) {
			closed_list.erase(it_pair.first);
			closed_list.insert(q);
		}

		for (const std::pair<point_i, direction>& pt : possible_children) {
			node child = make_child_node(q, pt);
			if (child.pos.x < 0 || child.pos.y < 0) {
				continue;
			}

			if (child.pos == destination) {
				ending_node = child;
				break;
			}

			if (m_tiles[static_cast<unsigned>(child.pos.x)][static_cast<unsigned>(child.pos.y)] != tiles::walkable) {
				if (std::isinf(wall_crossing_penalty) && !std::signbit(wall_crossing_penalty)) {
					continue;
				}
				child.heur += wall_crossing_penalty;
			}

			{
				auto it = std::find(open_list.begin(), open_list.end(), child);
				if (it != open_list.end()) {
					if (it->dist <= child.dist) {
						continue;
					}
					open_list.erase(it);
					open_list.insert(child);
				}
			}

			auto it = closed_list.find(child);
			if (it != closed_list.end() && it->dist <= q.dist + 1) {
				continue;
			}
			open_list.insert(child);
		}

	} while (!ending_node && !open_list.empty());


	if (ending_node) {
		std::vector<direction> reversed;
		node n = *ending_node;
		while (n.pos != source) {
			reversed.push_back(n.parent);
			n.pos.translate_fixed(-n.parent, 1);
			auto it = closed_list.find(n);
			assert(it != closed_list.end());
			n = *it;
		}
		ans.reserve(reversed.size());
		std::copy(reversed.rbegin(), reversed.rend(), std::back_inserter(ans));
	}
	return ans;
}

std::vector<dungeep::point_i>
map::path_to_pt(const dungeep::point_i& source, const dungeep::point_i& destination, float wall_crossing_penalty) const {
	std::vector<dungeep::direction> dirs = path_to(source, destination, wall_crossing_penalty);
	std::vector<dungeep::point_i> poss;
	poss.reserve(dirs.size() + 1);
	poss.push_back(source);
	for (dungeep::direction dir : dirs) {
		poss.push_back(poss.back());
		poss.back().translate_fixed(dir, 1);
	}
	return poss;
}
