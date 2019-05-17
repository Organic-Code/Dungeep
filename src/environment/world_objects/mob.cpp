///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2019, Lucas Lazare                                                                                                     ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,         ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  is furnished to do so, subject to the following conditions:                                                                        ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,        ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <json/json.h>
#include <utils/resource_keys.hpp>
#include <environment/world_objects/mob.hpp>
#include <spdlog/spdlog.h>

#include "environment/world_objects/mob.hpp"
#include "utils/resource_manager.hpp"
#include "iconned/fixed.hpp"
#include "iconned/dynamic.hpp"

mob::mob(const resources::creature_info& infos, unsigned int level) noexcept :
		creature(infos.name)
{
	current_direction = dungeep::direction::none;

	const resources::creature_info& me = resources::manager->read_creature(name);
	max_health = std::max(static_cast<int>(me.base_hp + me.hp_per_level * level), 1);
	attack_power = std::max(static_cast<int>(me.base_physical_power + me.physical_power_per_level * level), 1);
	armor = std::max(static_cast<int>(me.base_armor + me.armor_per_level * level), 1);
	resist = std::max(static_cast<int>(me.base_resist + me.resist_per_level * level), 1);
	move_speed = std::max(static_cast<int>(me.base_move_speed + me.move_speed_per_level * level), 1);
	crit_chance = std::max(static_cast<int>(me.base_crit_chance + me.crit_chance_per_level * level), 1);
}

void mob::tick(world_proxy& /*world*/) noexcept {
	// TODO
}

int mob::sleep() noexcept {
	return 0;
}

mob::~mob() {
	spdlog::trace("Mob: {} killed.", name);
}
