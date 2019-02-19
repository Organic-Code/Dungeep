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

#include <json/json.h>
#include <resource_manager.hpp>
#include <iconned/dynamic.hpp>
#include <world_objects/chest.hpp>

#include "world_objects/item.hpp"
#include "utils/random.hpp"

std::unique_ptr<item> item::generate_rand(chest_level chest_level) {
	// Todo: rareté (cf paramètre)
	// Todo: limiter à 'un' exemplaire de chaque objet (pour chaque joueur) [en paramètre (inventaire de joueur) ?]

	using keys = resources::item_keys;
	const Json::Value& item_props = resources::manager.read_item(static_cast<unsigned int>(dungeep::random_engine() % resources::manager.get_item_count()));

	std::string name = item_props[keys::name].asString();
	fixed_effect::defense def{item_props[keys::hp].asInt(), item_props[keys::armor].asInt(), item_props[keys::resist].asInt()};
	fixed_effect::attack atk{item_props[keys::attack].asInt(), item_props[keys::mag_atk].asInt(), item_props[keys::attack_speed].asInt()};
	fixed_effect::critics crits{item_props[keys::phy_crit_chance].asInt(), item_props[keys::mag_crit_chance].asInt()};
	fixed_effect::misc m{item_props[keys::move_speed].asFloat(), item_props[keys::armor_pen].asInt(), item_props[keys::resist_pen].asInt()};

	std::unique_ptr<fixed_effect> ptr;
	if (item_props[keys::is_dynamic].asBool()) {
		ptr = dynamic_effect::find_by_name(std::move(name), def, atk, crits, m, item_props);
	} else {
		ptr = std::make_unique<fixed_effect>(std::move(name), def, atk, crits, m);
	}

	return std::make_unique<item>(std::move(ptr));
}

