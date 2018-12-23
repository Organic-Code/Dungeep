#ifndef DUNGEEP_FIXED_HPP
#define DUNGEEP_FIXED_HPP

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

#include <memory>

#include "iconned/iconned.hpp"

class fixed_effect : public iconned {

public:
	std::string_view get_tooltip() const noexcept override;
	void print_at(sf::RenderWindow&) const noexcept override;

	int physical_crit_chance() const noexcept;
	int magical_crit_chance() const noexcept;
	int physical_damage_bonus() const noexcept;
	int magical_damage_bonus() const noexcept;
	float move_speed_bonus() const noexcept;
	int attack_speed_bonus() const noexcept; // percent
	int armor_bonus() const noexcept;
	int resist_bonus() const noexcept;
	int hp_bonus() const noexcept;
	int ignored_armor() const noexcept;
	int ignored_resist() const noexcept;

	virtual bool is_dynamic() const noexcept {
		return false;
	}
};

// placeholder for fixed_effect, clearer name for items
struct fixed_effect_item {
	fixed_effect effect;

	int physical_crit_chance() const noexcept {
		return effect.physical_crit_chance();
	}
	int magical_crit_chance() const noexcept {
		return effect.magical_crit_chance();
	}
	int physical_damage_bonus() const noexcept {
		return effect.physical_damage_bonus();
	}
	int magical_damage_bonus() const noexcept {
		return effect.magical_damage_bonus();
	}
	float move_speed_bonus() const noexcept {
		return effect.move_speed_bonus();
	}
	int attack_speed_bonus() const noexcept {
		return effect.attack_speed_bonus();
	}
	int armor_bonus() const noexcept {
		return effect.armor_bonus();
	}
	int resist_bonus() const noexcept {
		return effect.resist_bonus();
	}
	int hp_bonus() const noexcept {
		return effect.hp_bonus();
	}

	bool is_dynamic() const noexcept {
		return false;
	}
};

#endif //DUNGEEP_FIXED_HPP
