#ifndef DUNGEEP_RESOURCE_KEYS_HPP
#define DUNGEEP_RESOURCE_KEYS_HPP

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

// using const char* because jsoncpp does not handle std::string_view anyway
namespace keys {
	namespace creature {
		constexpr const char * type                        = "type"; // cf values::creature::type
		constexpr const char * hp                          = "base hp";
		constexpr const char * hp_pl                       = "hp per level";
		constexpr const char * phys_power                  = "base physical power";
		constexpr const char * phys_power_pl               = "physical power per level";
		constexpr const char * armor                       = "base armor";
		constexpr const char * armor_pl                    = "armor per level";
		constexpr const char * resist                      = "base resist";
		constexpr const char * resist_pl                   = "resist per level";
		constexpr const char * crit                        = "base crit chance";
		constexpr const char * crit_pl                     = "crit chance per level";
		constexpr const char * move_speed                  = "base move speed";
		constexpr const char * move_speed_pl               = "move speed per level";
		namespace spawner {
			constexpr const char * burst_duration          = "spawner spawn count";
			constexpr const char * burst_inner_interval    = "spawner spawn interval";
			constexpr const char * burst_interval          = "spawner spawn long interval";
		}
		namespace map {
			constexpr const char * list                    = "available for maps";
			constexpr const char * min_level               = "minimum level";
			constexpr const char * max_level               = "maximum level";
			constexpr const char * populate_factor         = "populate factor";
		}
	}

	namespace  player {
		constexpr const char * mana                        = "base mana";
		constexpr const char * mana_pl                     = "mana per level";
		constexpr const char * mag_power                   = "base magical power";
		constexpr const char * mag_power_pl                = "magical power per level";
	}

	namespace item {
		constexpr const char * name                        = "name";
		constexpr const char * is_dynamic                  = "dynamic";
		constexpr const char * phy_crit_chance             = "physical crit. chance";
		constexpr const char * mag_crit_chance             = "magical crit. chance";
		constexpr const char * attack                      = "attack power";
		constexpr const char * mag_atk                     = "magic power";
		constexpr const char * move_speed                  = "move speed";
		constexpr const char * attack_speed                = "attack speed";
		constexpr const char * armor                       = "armor";
		constexpr const char * resist                      = "resist";
		constexpr const char * hp                          = "hit points";
		constexpr const char * armor_pen                   = "armor pen.";
		constexpr const char * resist_pen                  = "resist pen.";
		constexpr const char * cooldown                    = "cooldown";
		constexpr const char * armor_pen_percent           = "armor pen. (%)";
		constexpr const char * resist_pen_percent          = "resist pen. (%)";
		constexpr const char * attack_inc_percent          = "attack power dealt (%)";
		constexpr const char * magic_inc_percent           = "magic power dealt (%)";
		constexpr const char * true_damage_inc_percent     = "true damage dealt (%)";
		constexpr const char * phy_damage_in_inc           = "physical damage taken (%)";
		constexpr const char * mag_damage_in_inc           = "magical damage taken (%)";
	}

	namespace text {
		constexpr const char * lang_name                   = "language name";
		constexpr const char * autoscroll                  = "terminal: autoscroll checkbox";
		constexpr const char * clear                       = "terminal: clear button";
		constexpr const char * autowrap                    = "terminal: autowrap long lines";
		constexpr const char * log_level                   = "terminal: log level selector";
		constexpr const char * trace                       = "terminal: log level: trace";
		constexpr const char * debug                       = "terminal: log level: debug";
		constexpr const char * info                        = "terminal: log level: info";
		constexpr const char * warning                     = "terminal: log level: warning";
		constexpr const char * error                       = "terminal: log level: error";
		constexpr const char * critical                    = "terminal: log level: critical";
		constexpr const char * none                        = "terminal: log level: none";
	}

	namespace config {
		constexpr const char * language                    = "language file";
	}

	namespace map {
		namespace categories {
			constexpr const char * zones                   = "zones properties";
			constexpr const char * rooms                   = "rooms properties";
			constexpr const char * holes                   = "holes properties";
			constexpr const char * halls                   = "halls properties";
		}
		namespace sizes {
			constexpr const char * width                   = "width";
			constexpr const char * height                  = "height";
		}
		namespace zones {
			constexpr const char * avg_size                = "average size";
			constexpr const char * size_deviation          = "size deviation";
			constexpr const char * borders_fuzzinness      = "borders fuzziness";
			constexpr const char * borders_fuzzy_deviation = "borders fuzzy deviation";
			constexpr const char * borders_fuzzy_distance  = "borders fuzzy distance";
			constexpr const char * min_height              = "minimum height";
			constexpr const char * max_height              = "maximum height";
			constexpr const char * avg_rooms_n             = "average room count";
			constexpr const char * rooms_n_dev             = "room count deviation";
			constexpr const char * avg_holes_n             = "average hole count";
			constexpr const char * holes_n_dev             = "hole count deviation";
		}
		namespace halls {
			constexpr const char * curliness               = "curliness";
			constexpr const char * curly_min_distance      = "curly minimum distance";
			constexpr const char * curly_segment_avg_size  = "curly segment average size";
			constexpr const char * curly_segment_size_dev  = "curly segment size deviation";
			constexpr const char * avg_width               = "average width";
			constexpr const char * width_dev               = "width deviation";
			constexpr const char * min_width               = "minimum width";
			constexpr const char * max_width               = "maximum width";
		}
		namespace chests {
			constexpr const char * rubbish_min             = "min. rubbish chest";
			constexpr const char * rubbish_max             = "max. rubbish chest";
			constexpr const char * wooden_min              = "min. wooden chest";
			constexpr const char * wooden_max              = "max. wooden chest";
			constexpr const char * iron_min                = "min. iron chest";
			constexpr const char * iron_max                = "max. iron chest";
			constexpr const char * magic_min               = "min. magic chest";
			constexpr const char * magic_max               = "max. magic chest";
		}
	}

	namespace sprites {
		constexpr const char * creatures                    = "creatures";
		constexpr const char * maps                        = "maps";
		namespace geometry {
			constexpr const char * x                       = "x";
			constexpr const char * y                       = "y";
			constexpr const char * width                   = "width";
			constexpr const char * height                  = "height";
		}
		namespace orientation {
			constexpr const char * top                     = "top";
			constexpr const char * top_right               = "top right";
			constexpr const char * right                   = "right";
			constexpr const char * bot_right               = "bot right";
			constexpr const char * bot                     = "bot";
			constexpr const char * bot_left                = "bot left";
			constexpr const char * left                    = "left";
			constexpr const char * top_left                = "top left";
		}

		namespace tiles {
			constexpr const char * hole                    = "hole";
			constexpr const char * walkable                = "walkable";
			constexpr const char * empty_space             = "empty space";
			constexpr const char * wall                    = "wall";
		}
	}
}

namespace values {
	namespace creature {
		namespace type {
			constexpr const char * player                  = "player";
			constexpr const char * creep                   = "creep";
			constexpr const char * creep_boss              = "creep boss";
		}
		constexpr const char * spawner_suffix              = "_spawner";
	}
}

#endif // DUNGEEP_RESOURCE_KEYS_HPP