#ifndef DUNGEEP_CREATURE_HPP
#define DUNGEEP_CREATURE_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018-2019, Lucas Lazare                                                                                                ///
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

#include <vector>

#include "iconned/fixed.hpp"
#include "iconned/dynamic.hpp"
#include "dynamic_object.hpp"

namespace sf {
	class Sprite;
}

class creature : public dynamic_object {
public:

	explicit creature(const std::string& name) noexcept;

	virtual void magical_hit(int damage, int resist_ignore) noexcept;

	virtual void physical_hit(int damage, int armor_ignore) noexcept;

	void add_effect(std::unique_ptr<fixed_effect>&& effect) noexcept;

	void remove_effect(fixed_effect*) noexcept;

	// returns the number of forced non-sleep ticks
	virtual int sleep() noexcept = 0;

	virtual void true_hit(int damage) noexcept;

	void print(sf::RenderWindow& rw) const noexcept override;

	virtual int get_armor() const noexcept {
		return armor;
	}

	virtual int get_resist() const noexcept {
		return resist;
	}

	virtual int get_hp() const noexcept {
		return current_health;
	}

	virtual int get_max_hp() const noexcept {
		return max_health;
	}

	virtual void heal(int heal) noexcept {
		current_health = std::min(get_hp() + heal, get_max_hp());
	}

protected:

	// with n armor/resist, you have a (n/10)% effective hp boost
	static int compute_damage_reduction(int damage, int defense) noexcept {
		if (defense >= 0) {
			return damage * 1000 / (1000 + defense);
		} else {
			return damage * 2 - damage * 1000 / (1000 - defense);
		}
	}

	int current_health{};
	int max_health{};
	int armor{};
	int resist{};
	int attack_power{};
	int crit_chance{};
	int move_speed{};

	dungeep::direction current_direction{};

	std::vector<std::unique_ptr<fixed_effect>> fixed_buffs{};
	std::vector<std::unique_ptr<dynamic_effect>> dynamic_buffs{};

	std::array<sf::Sprite, static_cast<unsigned>(dungeep::direction::none)>& sprites; // TODO: vector of array / array of vector (animations)
};

#endif //DUNGEEP_CREATURE_HPP
