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
		constexpr const char * type                    = "type"; // cf values::creature::type
		constexpr const char * hp                      = "base hp";
		constexpr const char * hp_pl                   = "hp per level";
		constexpr const char * phys_power              = "base physical power";
		constexpr const char * phys_power_pl           = "physical power per level";
		constexpr const char * armor                   = "base armor";
		constexpr const char * armor_pl                = "armor per level";
		constexpr const char * resist                  = "base resist";
		constexpr const char * resist_pl               = "resist per level";
		constexpr const char * crit                    = "base crit chance";
		constexpr const char * crit_pl                 = "crit chance per level";
		constexpr const char * move_speed              = "base move speed";
		constexpr const char * move_speed_pl           = "move speed per level";
		namespace spawner {
			constexpr const char * burst_duration      = "spawner spawn count";
			constexpr const char * burst_interval      = "spawner spawn interval";
		}
		namespace map {
			constexpr const char * list                = "available for maps";
			constexpr const char * min_level           = "minimum level";
			constexpr const char * max_level           = "maximum level";
		}
	}
	namespace  player {
		constexpr const char * mana                    = "base mana";
		constexpr const char * mana_pl                 = "mana per level";
		constexpr const char * mag_power               = "base magical power";
		constexpr const char * mag_power_pl            = "magical power per level";
	}
	namespace item {
		constexpr const char * name                    = "name";
		constexpr const char * is_dynamic              = "dynamic";
		constexpr const char * phy_crit_chance         = "physical crit. chance";
		constexpr const char * mag_crit_chance         = "magical crit. chance";
		constexpr const char * attack                  = "attack power";
		constexpr const char * mag_atk                 = "magic power";
		constexpr const char * move_speed              = "move speed";
		constexpr const char * attack_speed            = "attack speed";
		constexpr const char * armor                   = "armor";
		constexpr const char * resist                  = "resist";
		constexpr const char * hp                      = "hit points";
		constexpr const char * armor_pen               = "armor pen.";
		constexpr const char * resist_pen              = "resist pen.";
		constexpr const char * cooldown                = "cooldown";
		constexpr const char * armor_pen_percent       = "armor pen. (%)";
		constexpr const char * resist_pen_percent      = "resist pen. (%)";
		constexpr const char * attack_inc_percent      = "attack power dealt (%)";
		constexpr const char * magic_inc_percent       = "magic power dealt (%)";
		constexpr const char * true_damage_inc_percent = "true damage dealt (%)";
		constexpr const char * phy_damage_in_inc       = "physical damage taken (%)";
		constexpr const char * mag_damage_in_inc       = "magical damage taken (%)";
	}
	namespace text {
		constexpr const char * lang_name               = "language name";
		constexpr const char * dflt_lang_name          = "default language name";
	}
	namespace config {
		constexpr const char * language                = "language file";
	}
}

namespace values {
	namespace creature {
		namespace type {
			constexpr const char * player              = "player";
			constexpr const char * creep               = "creep";
			constexpr const char * creep_boss          = "creep boss";
		}
		constexpr const char * spawner_suffix          = "_spawner";
	}
}

#endif // DUNGEEP_RESOURCE_KEYS_HPP