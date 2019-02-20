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

#include "utils/resource_manager.hpp"
#include "environment/world_objects/mob.hpp"
#include "environment/world.hpp"

void world::generate_next_level() {
	const std::vector<std::string>& map_list = resources::manager.get_map_list();
	const std::string& map_name = map_list[shared_random() % map_list.size()];
	auto map_properties = resources::manager.get_map(map_name);
	std::vector<map::map_area> room_list = shared_map.generate(
			  std::get<map::size_type>(map_properties)
			, std::get<std::vector<room_gen_properties>>(map_properties)
			, std::get<hallway_gen_properties>(map_properties)
    );

	auto size = shared_map.size();
	dungeep::area_f map_area{{0.f, 0.f}, {static_cast<float>(size.width), static_cast<float>(size.height)}};

	dynamic_objects = decltype(dynamic_objects)(map_area);
	static_objects = decltype(static_objects)(map_area);

	std::vector<resources::creature_info> mobs = resources::manager.get_creatures_for_level(current_level, map_name);
	assert(!mobs.empty());

	unsigned int density = mobs_per_100_tiles();

	auto try_gen_pos = [this](const map::map_area& room, dungeep::area_f& generated_area, dungeep::dim_uc dim) {
		unsigned int i = 3;
		bool valid_pos;
		do {
			generated_area.top_left.x = static_cast<float>(shared_random() % (room.width - dim.x) + room.x);
			generated_area.top_left.y = static_cast<float>(shared_random() % (room.height - dim.y) + room.y);
			generated_area.bot_right.x = generated_area.top_left.x + static_cast<float>(dim.x);
			generated_area.bot_right.y = generated_area.top_left.y + static_cast<float>(dim.y);
			valid_pos = shared_map[generated_area.top_left] == tiles::walkable && shared_map[generated_area.bot_right] == tiles::walkable;
		} while (!valid_pos && i--);
		return valid_pos;
	};

	dungeep::area_f mob_pos;
	for (const map::map_area& room : room_list) {
		for (unsigned int mob_count = density * room.height * room.width / 100 ; mob_count != 0 ; --mob_count) {
			const resources::creature_info& selected_mob = mobs[shared_random() % mobs.size()];
			if (try_gen_pos(room, mob_pos, selected_mob.size)) {
				dynamic_objects.emplace(mob_pos, std::make_unique<mob>(mobs[shared_random() % mobs.size()], current_level));
			}
		}
	}

	// TODO: objets statiques (coffres, boutons, …)

	// TODO: haut-faits
	// TODO: sortie et entrée du niveau
}