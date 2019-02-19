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

#include "environment/world_objects/mob_spawner.hpp"
#include "environment/world_objects/mob.hpp"
#include "environment/world_proxy.hpp"
#include "utils/resource_manager.hpp"

mob_spawner::mob_spawner(const resources::creature_info& infos_, int level_) noexcept
		: creature(infos_.name + resources::creature_values::spawner_suffix)
		, infos(infos_)
		, level(level_)
{
	const Json::Value& creature = resources::manager.read_creature(infos_.name);
	max_cooldown = creature.[resources::creature_keys::spawner::burst_interval].asUInt();
	max_creature_count = creature.[resources::creature_keys::spawner::burst_duration].asUInt();

}

void mob_spawner::tick(world_proxy& world) noexcept {
	if (creature_count < max_creature_count) {
		if (cooldown == 0u) {
			cooldown = max_cooldown;
			world.create_entity(std::make_unique<mob>(infos, level));
			++creature_count;
		} else {
			cooldown--;
		}
	}
}

int mob_spawner::sleep() noexcept {
	creature_count = 0u;
	cooldown = 0u;
	return 0;
}
