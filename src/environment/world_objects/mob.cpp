///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018, Lucas Lazare                                                                                                     ///
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

#include "environment/world_objects/mob.hpp"
#include "utils/resource_manager.hpp"
#include "iconned/fixed.hpp"
#include "iconned/dynamic.hpp"

mob::mob(std::string name_, int level) noexcept :
		creature(name_),
		name{std::move(name_)},
		current_direction{dungeep::direction::none}
{
	using ck = resources::creature_keys;

	const Json::Value& me = resources::manager.read_creature(name);
	max_health = me.get(ck::hp, 0).asInt() + me.get(ck::hp_pl, 0).asInt() * level;
	attack_power = me.get(ck::phys_power, 0).asInt() + me.get(ck::phys_power_pl, 0).asInt() * level;
	armor = me.get(ck::armor, 0).asInt() + me.get(ck::armor_pl, 0).asInt() * level;
	resist = me.get(ck::resist, 0).asInt() + me.get(ck::resist_pl, 0).asInt() * level;
	move_speed = me.get(ck::move_speed,0).asInt() + me.get(ck::move_speed_pl, 0).asInt() * level;
	crit_chance = me.get(ck::crit, 0).asInt() + me.get(ck::crit_pl, 0).asInt() * level;
}

void mob::tick(world_proxy& world) noexcept {
	// TODO
}

int mob::sleep() noexcept {
	return 0;
}
