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

#include <random>
#include <algorithm>
#include <unordered_set>
#include <set>
#include <environment/map.hpp>
#include <chrono>
#include <utils/quadtree.hpp>
#include <spdlog/spdlog.h>

#include "utils/random.hpp"
#include "environment/map.hpp"

std::vector<map::map_area> map::generate(size_type size, const std::vector<room_gen_properties>& rooms_properties,
                                    const hallway_gen_properties& hgp) {
	using std::chrono::duration_cast;
	using std::chrono::system_clock;
	using std::chrono::milliseconds;

	fuzzy_generation_time = milliseconds{0};
	expected_hole_count = expected_room_count = actual_hole_count = actual_room_count = 0;

	auto starting_tp = system_clock::now();

	m_tiles = {size.width, std::vector<tiles>{size.height, tiles::empty_space}};
	assert(size.width > 0);
	assert(!rooms_properties.empty());

	std::vector<map_area> rooms;
	rooms.reserve(rooms_properties.size() * static_cast<unsigned long>(rooms_properties[0].avg_rooms_n));

	auto room_starting_tp = system_clock::now();
	spdlog::debug("[Map] - Generating rooms.");
	for (const room_gen_properties& rp : rooms_properties) {
		auto rooms_n = gen_positive(rp.avg_rooms_n, rp.rooms_n_dev);
		auto holes_n = gen_positive(rp.avg_holes_n, rp.holes_n_dev);

		expected_room_count += static_cast<unsigned>(rooms_n - 0.3f) + 2;
		expected_hole_count += static_cast<unsigned>(holes_n);

		for (auto i = 0 ; static_cast<float>(i) < (rooms_n - 0.3f) ; ++i) {

			unsigned int hfr = std::min(
					static_cast<unsigned int>(holes_n),
					static_cast<unsigned int>(gen_positive(holes_n / (rooms_n - static_cast<float>(i)), 2.5f))
			);

			holes_n -= static_cast<float>(hfr);
			map_area room = generate_holed_room(rp, hfr);
			if (room.x != 0 && room.y != 0) {
				++actual_room_count;
				rooms.push_back(room);
			}
		}
		map_area room = generate_holed_room(rp, static_cast<unsigned int>(holes_n));
		if (room.x != 0 && room.y != 0) {
			++actual_room_count;
			rooms.push_back(room);
		}
	}
	spdlog::debug("[Map] - Generated {} rooms.", actual_room_count);
	rooms_generation_time = duration_cast<milliseconds>(system_clock::now() - room_starting_tp);

	auto halls_tp = system_clock::now();
	ensure_pathing(rooms, hgp);
	halls_generation_time = duration_cast<milliseconds>(system_clock::now() - halls_tp);

	total_generation_time = duration_cast<milliseconds>(system_clock::now() - starting_tp);

	return rooms;
}

void map::ensure_pathing(const std::vector<map_area>& rooms, const hallway_gen_properties& properties) {

	spdlog::debug("[Map] - Generating hallways.");

	struct collider {
		const dungeep::area_f& hitbox() const {
			return hitbox_;
		}

		dungeep::area_f hitbox_;
		dungeep::point_ui room_center;
	};
	dungeep::quadtree<collider, dungeep::quadtree_dynamics::static_children> qt(
			dungeep::area_f{
					dungeep::point_f{0.f, 0.f},
					dungeep::point_f{static_cast<float>(size().width), static_cast<float>(size().height)}
			},
			3,
			5
	);

	std::vector<dungeep::point_ui> rooms_center;
	rooms_center.reserve(rooms.size());
	for (const map_area& room : rooms) {
		rooms_center.emplace_back(room.x + room.width / 2, room.y + room.height / 2);
	}

	float avg_distance = 0.f;
	std::vector<float> distances;
	distances.reserve(rooms_center.size() * (rooms_center.size() - 1));
	for (const dungeep::point_ui& center_1 : rooms_center) {
		for (const dungeep::point_ui& center_2 : rooms_center) {
			float distance = static_cast<float>((dungeep::point_i{center_1} - dungeep::point_i{center_2}).length());
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

	avg_distance /= static_cast<float>(rooms_center.size() * (rooms_center.size() - 1));
	std::sort(distances.begin(), distances.end());

	float selected_distance = std::max(distances[distances.size() / 30], avg_distance / 30.f);
	for (const dungeep::point_ui& room : rooms_center) {
		auto x = static_cast<float>(room.x);
		auto y = static_cast<float>(room.y);
		dungeep::area_f htbox{
				dungeep::point_f{x - selected_distance, y - selected_distance},
				dungeep::point_f{x + selected_distance, y + selected_distance}
		};

		qt.visit(htbox, [this, &properties, &room, &selected_distance](auto it) {
			if (dungeep::random_engine() % 2) {
				if (path_to(dungeep::point_i(room), dungeep::point_i(it->room_center), std::numeric_limits<float>::infinity(), static_cast<int>(selected_distance * 2.f)).empty()) {
					ensure_tworoom_path(room, it->room_center, properties);
				}
			}
		});
	}

	for (const dungeep::point_ui& room : rooms_center) {
		auto x = static_cast<float>(room.x);
		auto y = static_cast<float>(room.y);
		dungeep::area_f htbox{
				dungeep::point_f{x - static_cast<float>(size().width) / 10.f, y - static_cast<float>(size().height) / 10.f},
				dungeep::point_f{x + static_cast<float>(size().width) / 10.f, y + static_cast<float>(size().height) / 10.f}
		};

		qt.visit(htbox, [this, &properties, &room, &selected_distance](auto it) {
			if (path_to(dungeep::point_i(room), dungeep::point_i(it->room_center), std::numeric_limits<float>::infinity(), static_cast<int>(selected_distance * 2.f)).empty()) {
				ensure_tworoom_path(room, it->room_center, properties);
			}
		});
	}

	for (auto i1 = qt.begin(), i2 = std::next(i1) ; !i2.is_at_end() && !i1.is_at_end() ; ++i1, ++i2) {
		if (dungeep::random_engine() % 3) {
			if (path_to(dungeep::point_i(i1->room_center), dungeep::point_i(i2->room_center),
			            std::numeric_limits<float>::infinity()).empty()) {
				ensure_tworoom_path(i1->room_center, i2->room_center, properties);
			}
		}
	}

	spdlog::debug("[Map] - Generating hallways: done.");
}

void map::ensure_tworoom_path(const dungeep::point_ui& r1_center, const dungeep::point_ui& r2_center, const hallway_gen_properties& properties) {

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

	if (static_cast<float>((dep - arr).length()) >= properties.curly_min_distance) {
		dungeep::normal_distribution curliness(0.f, std::max(properties.curliness, 0.001f));
		dungeep::normal_distribution curly_size(properties.curly_segment_avg_size, std::max(properties.curly_segment_size_dev, 0.001f));
		do {
			point_i translate = arr - stop_offs.back();
			float segment_size = std::max(curly_size(dungeep::random_engine), properties.curly_min_distance);
			float segment_angle = curliness(dungeep::random_engine) * static_cast<float>(M_PI) / 4;

			translate.rotate(segment_angle);
			translate.scale_to(segment_size);

			stop_offs.push_back(translate + stop_offs.back());

		} while (static_cast<float>((stop_offs.back() - arr).length()) >= properties.curly_min_distance);
	}

	stop_offs.push_back(arr);

	dungeep::normal_distribution width(properties.avg_width, std::max(properties.width_dev, 0.001f));
	auto w = static_cast<int>(std::clamp(static_cast<unsigned int>(width(dungeep::random_engine)), properties.min_width, properties.max_width));
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

map::map_area map::generate_holed_room(const room_gen_properties& rp, unsigned int hole_count) {
	bool failed;
	int fail_count = 0;

	dungeep::point_ui room_dim{}, room_pos{};

	do {
		failed = false;
		room_dim = generate_zone_dimensions(rp.rooms_properties);
		room_pos = find_zone_filled_with(room_dim, tiles::empty_space);
		if (room_pos.x == 0 && room_pos.y == 0) {
			failed = true;
			++fail_count;
		}
	} while (failed && fail_count < 30);

	if (failed) {
		return {0, 0, 0, 0};
	}

	map_area room_area{room_pos.x, room_pos.y, room_dim.x, room_dim.y};
	generate_tiles(rp.rooms_properties, room_area, tiles::walkable);

	for (auto i = 0u ; i < hole_count ; ++i) {
		fail_count = 0;
		do {
			dungeep::point_ui hole_dim = generate_zone_dimensions(rp.holes_properties);
			hole_dim.x = std::min(hole_dim.x, room_area.width - 1);
			hole_dim.y = std::min(hole_dim.y, room_area.height - 1);
			dungeep::point_ui hole_pos = find_zone_filled_with(hole_dim, tiles::walkable, room_area);
			if (hole_pos.x == 0 && hole_pos.y == 0) {
				++fail_count;
				failed = true;
			} else {
				generate_tiles(rp.holes_properties, {hole_pos.x, hole_pos.y, hole_dim.x, hole_dim.y}, tiles::hole);
				++actual_hole_count;
				failed = false;
			}
		} while (failed && fail_count < 10);
	}

	return room_area;
}

void map::generate_tiles(const zone_gen_properties& rp, map_area tiles_area, tiles tile) {
	dungeep::normal_distribution zone_fuzziness(0.f, std::max(rp.borders_fuzzy_deviation, 0.001f));

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
	add_fuzziness(generated_room, rp, ar, tile, zone_fuzziness);
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
                        tiles tile, dungeep::normal_distribution<float>& zone_fuzziness) {

	float current_delta = 0.f;
	float delta_step = 0.f;

	auto generate_fuzziness = [&](unsigned int min, unsigned int max, float delta_max, auto&& assigner) {

		for (auto i = min; i < max ; ++i) {
			if ((i % rp.borders_fuzzy_distance) == (min % rp.borders_fuzzy_distance)) {
				float target_delta = std::clamp(zone_fuzziness(dungeep::random_engine) * rp.borders_fuzzinness, -delta_max, delta_max);
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
map::find_zone_filled_with(dungeep::point_ui zone_dim, tiles tile, map_area area) const noexcept {

	assert(zone_dim.x < area.width);
	assert(zone_dim.y < area.height);
	assert(area.x + area.width  < size().width);
	assert(area.y + area.height < size().height);

	dungeep::uniform_int_distribution<unsigned int> uid_x(area.x, area.x + area.width);
	dungeep::uniform_int_distribution<unsigned int> uid_y(area.y, area.y + area.height);

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
		ans.x = uid_x(dungeep::random_engine);
		ans.y = uid_y(dungeep::random_engine);
	} while(!is_good_spot(ans) && fail_count++ < 100);

	if (fail_count >= 100) {
		ans = {0u, 0u};
	}

	return ans;
}

float map::gen_positive(float avg, float dev) {
	dungeep::normal_distribution dist{avg, std::max(dev, 0.001f)};
	float nbr = dist(dungeep::random_engine);
	return nbr > 0 ? nbr : 0.f;
}

dungeep::point_ui map::generate_zone_dimensions(const zone_gen_properties& zgp) {
	std::mt19937_64 random_engine;

	auto min_area = zgp.min_height;
	min_area *= min_area;
	auto max_area = zgp.max_height;
	max_area *= max_area;

	float room_area = std::clamp(gen_positive(zgp.avg_size, zgp.size_deviation), static_cast<float>(min_area), static_cast<float>(max_area));

	float avg_room_dim = static_cast<float>(zgp.max_height + zgp.min_height) / 2.f;
	dungeep::point_ui room_dim{};
	room_dim.x = std::clamp(static_cast<unsigned int>(gen_positive(avg_room_dim, 0.4f * avg_room_dim)), zgp.min_height, zgp.max_height);
	room_dim.y = static_cast<unsigned int>(room_area / static_cast<float>(room_dim.x));

	return room_dim;
}

std::vector<dungeep::direction> map::path_to(const dungeep::point_i& source, const dungeep::point_i& destination, float wall_crossing_penalty, int max_depth) const {
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
		int depth{0};

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
		return std::abs(std::hypot(static_cast<float>(destination.x - p.x), static_cast<float>(destination.y - p.y)));
	};
	auto hash = [](const node& n) noexcept {
		return n.pos.x + n.pos.y;
	};
	auto make_node = [&cost_of](const point_i& p, float distance) {
		return node{p, distance, cost_of(p)};
	};
	auto make_child_node = [&cost_of](const node& parent, const std::pair<point_i, direction>& translation) {
		dungeep::point_i new_pos = parent.pos + translation.first;

		return node{new_pos, parent.dist + std::hypot(static_cast<float>(translation.first.x), static_cast<float>(translation.first.y))
				, cost_of(new_pos), translation.second, parent.depth + 1};
	};

	std::unordered_set<node, decltype(hash)> closed_list{0, std::move(hash)};
	std::set<node> open_list{};

	open_list.emplace(make_node(source, 0.f));

	constexpr std::array<std::pair<point_i, direction>, 4> possible_children = {
			std::pair{point_i{ 0,-1}, direction::top      },
			std::pair{point_i{-1, 0}, direction::left     },
			std::pair{point_i{ 1, 0}, direction::right    },
			std::pair{point_i{ 0, 1}, direction::bot      },
	};

	constexpr std::array<std::pair<point_i, direction>, 4> possible_children_diag = {
			std::pair{point_i{ 1, 1}, direction::bot_right},
			std::pair{point_i{-1,-1}, direction::top_left},
			std::pair{point_i{ 1,-1}, direction::top_right},
			std::pair{point_i{-1, 1}, direction::bot_left}
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

		auto process_node = [&](node child) {

			if (child.pos == destination) [[unlikely]] {
				ending_node = child;
				return;
			}

			if (m_tiles[static_cast<unsigned>(child.pos.x)][static_cast<unsigned>(child.pos.y)] != tiles::walkable) {
				if (std::isinf(wall_crossing_penalty) && !std::signbit(wall_crossing_penalty)) {
					return;
				}
				child.dist += wall_crossing_penalty;
			}

			{
				auto it = std::find(open_list.begin(), open_list.end(), child);
				if (it != open_list.end()) {
					if (it->dist <= child.dist) {
						return;
					}
					open_list.erase(it);
					open_list.insert(child);
				}
			}

			auto it = closed_list.find(child);
			if (it != closed_list.end() && it->dist <= q.dist + 1) {
				return;
			}
			open_list.insert(child);
		};

		for (const std::pair<point_i, direction>& pt : possible_children) {
			node child = make_child_node(q, pt);
			if (child.pos.x < 0 || child.pos.y < 0 || static_cast<unsigned>(child.pos.x) >= size().width
			    || static_cast<unsigned>(child.pos.y) >= size().height || child.depth > max_depth) [[unlikely]] {
				continue;
			}
			process_node(child);
			if (ending_node) {
				break;
			}
		}

		for (const std::pair<point_i, direction>& pt : possible_children_diag) {
			node child = make_child_node(q, pt);
			if (child.pos.x < 0 || child.pos.y < 0 || static_cast<unsigned>(child.pos.x) >= size().width
			    || static_cast<unsigned>(child.pos.y) >= size().height || child.depth > max_depth) [[unlikely]] {
				continue;
			}

			if (m_tiles[static_cast<unsigned>(child.pos.x)][static_cast<unsigned>(q.pos.y)] != tiles::walkable
				|| m_tiles[static_cast<unsigned>(q.pos.x)][static_cast<unsigned>(child.pos.y)] != tiles::walkable) [[unlikely]] {
				continue;
			}
			process_node(child);
			if (ending_node) {
				break;
			}
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
