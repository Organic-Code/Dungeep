///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2019, Lucas Lazare                                                                                                     ///
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

#include "utils/random.hpp"
#include "environment/world_objects/chest.hpp"
#include "utils/constants.hpp"
#include "utils/resource_manager.hpp"
#include "environment/world_objects/mob.hpp"
#include "environment/world.hpp"

namespace {
	unsigned short chest_count_rand(dungeep::uniform_int_distribution<unsigned short>& dist, resources::chest_count cc, std::mt19937_64& rand) {
		dist.param(dungeep::uniform_int_distribution<unsigned short>::param_type{cc.min, cc.max});
		return dist(rand);
	}
}

void world::generate_next_level() {

	// retrieving map settings
	const std::unordered_map<std::string, resources::map_info>& map_list = resources::manager->get_map_list();
	const auto& selected_map = *std::next(map_list.begin(), static_cast<unsigned>(shared_random() % map_list.size()));

	const auto& map_properties = resources::manager->get_map(selected_map.first);
	std::vector<map::map_area> room_list = shared_map.generate(
			  map_properties.size
			, map_properties.rooms_props
			, map_properties.hallways_props
    );

	std::vector<std::pair<std::string_view, resources::mob_map_rinfo>> mobs = resources::manager->get_creatures_for_level(current_level, selected_map.first);

	if (mobs.empty()) {
		spdlog::error("No creature available for level {} on map {}. Aborting.", current_level, selected_map.first);
		std::exit(0);
	}

	// Setting Quadtrees working area
	auto size = shared_map.size();
	dungeep::area_f map_area{{0.f, 0.f}, {static_cast<float>(size.width), static_cast<float>(size.height)}};

	dynamic_objects = decltype(dynamic_objects)(map_area);
	static_objects = decltype(static_objects)(map_area);


	// TODO: objets statiques (boutons, …) (avant les mobs)
	dungeep::uniform_int_distribution<unsigned short> c_dist;
	put_n_chest(chest_level::rubbish, chest_count_rand(c_dist, map_properties.rubbish_chest, shared_random), room_list);
	put_n_chest(chest_level::wooden, chest_count_rand(c_dist, map_properties.wooden_chest, shared_random), room_list);
	put_n_chest(chest_level::magic, chest_count_rand(c_dist, map_properties.magic_chest, shared_random), room_list);
	put_n_chest(chest_level::iron, chest_count_rand(c_dist, map_properties.iron_chest, shared_random), room_list);



	unsigned int density = mobs_per_100_tiles();

	// Probability : sum of pop factors (preparing mob generation)
	const unsigned int total_pop_factor = std::accumulate(mobs.begin(), mobs.end(), 0u,
			[](unsigned int s, const auto& mob) {
				return s + mob.second.populate_factor;
			});
	dungeep::uniform_int_distribution mob_distribution{0u, total_pop_factor};


	// Placing mobs
	dungeep::area_f mob_pos;
	for (const map::map_area& room : room_list) {
		for (unsigned int mob_count = density * room.height * room.width / 100 ; mob_count != 0 ; --mob_count) {

			// selecting a random mob
			const unsigned int selected_mob_pop = mob_distribution(shared_random);
			unsigned int selected_mob_idx = 0;
			for (unsigned int sum = mobs[0].second.populate_factor ; selected_mob_idx < mobs.size() && sum < selected_mob_pop ; ++selected_mob_idx) {
				sum += mobs[selected_mob_idx].second.populate_factor;
			}

			// trying to place it
			const resources::creature_info& cinfo = resources::manager->read_creature(mobs[selected_mob_idx].first);
			if (try_gen_pos(room, mob_pos, cinfo.size)) {
				dynamic_objects.emplace(mob_pos, std::make_unique<mob>(cinfo, current_level));
			}
		}
	}

	// TODO: sortie et entrée du niveau
}

bool world::try_gen_pos(const map::map_area& room, dungeep::area_f& /* out */ generated_area, dungeep::dim_uc dim) {
	assert(room.x >= 0 && room.y >= 0);
	if (room.width <= dim.x || room.height <= dim.y) {
		return false;
	}

	unsigned int i = 5;
	bool valid_pos;
	do {
		generated_area.top_left.x = static_cast<float>(shared_random() % (room.width - dim.x) + room.x);
		generated_area.top_left.y = static_cast<float>(shared_random() % (room.height - dim.y) + room.y);
		generated_area.bot_right.x = generated_area.top_left.x + static_cast<float>(dim.x);
		generated_area.bot_right.y = generated_area.top_left.y + static_cast<float>(dim.y);
		valid_pos = true;
		for (auto j = static_cast<unsigned>(generated_area.top_left.x) ; j < generated_area.bot_right.x && valid_pos ; ++j) {
			for (auto k = static_cast<unsigned>(generated_area.top_left.y) ; k < generated_area.bot_right.y ; ++k) {
				if (shared_map[j][k] != tiles::walkable) {
					valid_pos = false;
					break;
				}
			}
		}
		if (valid_pos) {
			valid_pos = !static_objects.has_collision(generated_area);
		}

	} while (!valid_pos && i--);

	return valid_pos;
}


void world::put_n_chest(chest_level lvl, unsigned short count, const std::vector<map::map_area>& room_list) {
	dungeep::area_f location;

	while (count--) {
		unsigned int i = 0;
		do {
			map::map_area target_room = room_list[shared_random() % room_list.size()];
			if (try_gen_pos(target_room, location, constants::chests::size)) {
				static_objects.emplace(location, std::make_unique<chest>(lvl));
				return;
			}
		} while (++i < 5);
	}
}

