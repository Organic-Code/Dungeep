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

// NOT TO BE INCLUDED FROM ANYWHERE ASIDE RESOURCE MANAGER'S HEADER


// using const char* because jsoncpp does not handle std::string_view anyway
struct creature_keys {
	static constexpr const char* type                    = "type"; // cf creature_values::type
	static constexpr const char* hp                      = "base hp";
	static constexpr const char* hp_pl                   = "hp per level";
	static constexpr const char* phys_power              = "base physical power";
	static constexpr const char* phys_power_pl           = "physical power per level";
	static constexpr const char* armor                   = "base armor";
	static constexpr const char* armor_pl                = "armor per level";
	static constexpr const char* resist                  = "base resist";
	static constexpr const char* resist_pl               = "resist per level";
	static constexpr const char* crit                    = "base crit chance";
	static constexpr const char* crit_pl                 = "crit chance per level";
	static constexpr const char* move_speed              = "base move speed";
	static constexpr const char* move_speed_pl           = "move speed per level";
	struct spawner {
		static constexpr const char* burst_duration      = "spawner spawn count";
		static constexpr const char* burst_interval      = "spawner spawn interval";
	};
	struct map {
		static constexpr const char* list                = "available for maps";
		static constexpr const char* min_level           = "minimum level";
		static constexpr const char* max_level           = "maximum level";
	};
};
struct creature_values {
	struct type {
		static constexpr const char* player              = "player";
		static constexpr const char* creep               = "creep";
		static constexpr const char* creep_boss          = "creep boss";
	};
	static constexpr const char* spawner_suffix          = "_spawner";
};
struct player_keys {
	static constexpr const char* mana                    = "base mana";
	static constexpr const char* mana_pl                 = "mana per level";
	static constexpr const char* mag_power               = "base magical power";
	static constexpr const char* mag_power_pl            = "magical power per level";
};
struct item_keys {
	static constexpr const char* name                    = "name";
	static constexpr const char* is_dynamic              = "dynamic";
	static constexpr const char* phy_crit_chance         = "physical crit. chance";
	static constexpr const char* mag_crit_chance         = "magical crit. chance";
	static constexpr const char* attack                  = "attack power";
	static constexpr const char* mag_atk                 = "magic power";
	static constexpr const char* move_speed              = "move speed";
	static constexpr const char* attack_speed            = "attack speed";
	static constexpr const char* armor                   = "armor";
	static constexpr const char* resist                  = "resist";
	static constexpr const char* hp                      = "hit points";
	static constexpr const char* armor_pen               = "armor pen.";
	static constexpr const char* resist_pen              = "resist pen.";
	static constexpr const char* cooldown                = "cooldown";
	static constexpr const char* armor_pen_percent       = "armor pen. (%)";
	static constexpr const char* resist_pen_percent      = "resist pen. (%)";
	static constexpr const char* attack_inc_percent      = "attack power dealt (%)";
	static constexpr const char* magic_inc_percent       = "magic power dealt (%)";
	static constexpr const char* true_damage_inc_percent = "true damage dealt (%)";
	static constexpr const char* phy_damage_in_inc       = "physical damage taken (%)";
	static constexpr const char* mag_damage_in_inc       = "magical damage taken (%)";
};
struct text_keys {
	static constexpr const char* lang_name               = "language name";
	static constexpr const char* dflt_lang_name          = "default language name";
};
struct config_keys {
	static constexpr const char* language                = "language file";
};
