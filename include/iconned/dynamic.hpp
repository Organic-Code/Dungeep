#ifndef DUNGEEP_DYNAMIC_HPP
#define DUNGEEP_DYNAMIC_HPP

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

#include "iconned/fixed.hpp"

class player;

class dynamic_effect : public fixed_effect {
public:
	virtual int update_cooldown_once(int current_cooldown) noexcept = 0;
	virtual int update_input_physical_damage(int input) noexcept = 0;
	virtual int update_input_magical_damage(int input) noexcept = 0;
	virtual int update_input_true_damage(int input) noexcept = 0;
	virtual int update_output_physical_damage(int input) noexcept = 0;
	virtual int update_output_magical_damage(int input) noexcept = 0;
	virtual int update_output_true_damage(int input) noexcept = 0;
	virtual int ignored_armor(int target_armor) noexcept = 0;
	virtual int ignored_resist(int target_resist) noexcept = 0;

	virtual void tick(player&) noexcept = 0;

	bool is_dynamic() const noexcept final {
		return true;
	}
};

// placeholder for dynamic_effect, clearer name for items
struct dynamic_effect_item {
	std::unique_ptr<dynamic_effect> effect;

	int physical_crit_chance() const noexcept {
		return effect->physical_crit_chance();
	}
	int magical_crit_chance() const noexcept {
		return effect->magical_crit_chance();
	}
	int physical_damage_bonus() const noexcept {
		return effect->physical_damage_bonus();
	}
	int magical_damage_bonus() const noexcept {
		return effect->magical_damage_bonus();
	}
	float move_speed_bonus() const noexcept {
		return effect->move_speed_bonus();
	}
	int attack_speed_bonus() const noexcept {
		return effect->attack_speed_bonus();
	}
	int armor_bonus() const noexcept {
		return effect->armor_bonus();
	}
	int resist_bonus() const noexcept {
		return effect->resist_bonus();
	}
	int hp_bonus() const noexcept {
		return effect->hp_bonus();
	}

	int update_cooldown_once(int current_cooldown) noexcept {
		return effect->update_cooldown_once(current_cooldown);
	}
	int update_input_physical_damage(int input) noexcept {
		return effect->update_input_physical_damage(input);
	}
	int update_input_magical_damage(int input) noexcept {
		return effect->update_input_magical_damage(input);
	}
	int update_input_true_damage(int input) noexcept {
		return effect->update_input_true_damage(input);
	}
	int update_output_physical_damage(int input) noexcept {
		return effect->update_output_physical_damage(input);
	}
	int update_output_magical_damage(int input) noexcept {
		return effect->update_output_magical_damage(input);
	}
	int update_output_true_damage(int input) noexcept {
		return effect->update_output_true_damage(input);
	}
	int ignored_armor(int target_armor) noexcept {
		return effect->ignored_armor(target_armor);
	}
	int ignored_resist(int target_resist) noexcept {
		return effect->ignored_resist(target_resist);
	}

	void tick(player& p) noexcept {
		effect->tick(p);
	}

	bool is_dynamic() const noexcept {
		return false;
	}
};

#endif //DUNGEEP_DYNAMIC_HPP
