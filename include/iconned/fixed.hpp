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
#include <utils/geometry.hpp>

#include "iconned/iconned.hpp"

namespace details {
	struct defense {
		int hp;
		int armor;
		int resist;
	};

	struct attack {
		int physic;
		int magic;
		int speed;
	};

	struct critics {
		int physic;
		int magic;
	};

	struct misc {
		float move_speed;
		int armor_pen;
		int resist_pen;
	};
}

class fixed_effect : public iconned {
	// TODO: systèmes d'ensemble > rajout d'un effet dynamique "global" à l'équipement du premier ensemble ?
	//  classe spéciale d'ensemble qui génère par la suite un/des effets dynamiques ?
	// > info bulle à mettre à jour également

public:
	using defense = details::defense;
	using attack = details::attack;
	using critics = details::critics;
	using misc = details::misc;

	fixed_effect(std::string&& name_, defense def, attack atk, critics crit, misc m)
		: name(std::move(name_))
		, physical_crit_chance_(crit.physic)
		, magical_crit_chance_(crit.magic)
		, physical_damage_bonus_(atk.physic)
		, magical_damage_bonus_(atk.magic)
		, move_speed_bonus_(m.move_speed)
		, attack_speed_bonus_(atk.speed)
		, armor_bonus_(def.armor)
		, resist_bonus_(def.resist)
		, hp_bonus_(def.hp)
		, ignored_armor_(m.armor_pen)
		, ignored_resist_(m.resist_pen)
		{}

	std::string_view get_tooltip() const noexcept override { /* todo */ return {}; }
	void print_icon_at(sf::RenderWindow&, const dungeep::area_f&) const noexcept override { /* todo */ }

	virtual void print_effect_at(sf::RenderWindow&, const dungeep::area_f&) const noexcept {
		//TODO
	}
	bool has_printed_effect() {
		return false; // TODO
	}


	int physical_crit_chance() const noexcept {
		return physical_crit_chance_;
	}

	int magical_crit_chance() const noexcept {
		return magical_crit_chance_;
	}

	int physical_damage_bonus() const noexcept {
		return physical_damage_bonus_;
	}

	int magical_damage_bonus() const noexcept {
		return magical_damage_bonus_;
	}

	float move_speed_bonus() const noexcept {
		return move_speed_bonus_;
	}

	int attack_speed_bonus() const noexcept { // percent
		return attack_speed_bonus_;
	}

	int armor_bonus() const noexcept {
		return armor_bonus_;
	}

	int resist_bonus() const noexcept {
		return resist_bonus_;
	}

	int hp_bonus() const noexcept {
		return hp_bonus_;
	}

	int ignored_armor() const noexcept {
		return ignored_armor_;
	}

	int ignored_resist() const noexcept {
		return ignored_resist_;
	}


	virtual bool is_dynamic() const noexcept {
		return false;
	}



private:
	std::string name;

	int physical_crit_chance_;
	int magical_crit_chance_;
	int physical_damage_bonus_;
	int magical_damage_bonus_;
	float move_speed_bonus_;
	int attack_speed_bonus_;
	int armor_bonus_;
	int resist_bonus_;
	int hp_bonus_;
	int ignored_armor_;
	int ignored_resist_;

};

#endif //DUNGEEP_FIXED_HPP
