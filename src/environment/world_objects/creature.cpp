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
#include <algorithm>
#include <utility>
#include <environment/world_objects/creature.hpp>
#include <utils/resource_manager.hpp>


#include "environment/world_objects/creature.hpp"
#include "iconned/fixed.hpp"
#include "iconned/dynamic.hpp"


creature::creature(const std::string& name) noexcept
	: sprites{resources::manager.get_creature_sprite(name)}
{
}

void creature::magical_hit(int damage, int resist_ignore) noexcept {
	std::vector<std::unique_ptr<fixed_effect>> fixed_buffs{};
	std::vector<std::unique_ptr<dynamic_effect>> dynamic_buffs{}; // TODO -> pas les fixed_buffs parcontre [ils sont déjà cachés dans 'resist' à priori
	current_health -= compute_damage_reduction(damage, resist - resist_ignore);
}

void creature::physical_hit(int damage, int armor_ignore) noexcept {
	std::vector<std::unique_ptr<fixed_effect>> fixed_buffs{};
	std::vector<std::unique_ptr<dynamic_effect>> dynamic_buffs{}; // TODO -> pas les fixed_buffs parcontre [ils sont déjà cachés dans 'armor' à priori
	current_health -= compute_damage_reduction(damage, armor - armor_ignore);
}

void creature::add_effect(std::unique_ptr<fixed_effect>&& effect) noexcept {
	if (effect->is_dynamic()) {
		auto dyn_effect = dynamic_cast<dynamic_effect*>(effect.release());
		assert(dyn_effect);
		dynamic_buffs.emplace_back(dyn_effect);
	} else {
		fixed_buffs.emplace_back(std::move(effect));
	}
}

void creature::remove_effect(fixed_effect* effect) noexcept {

	auto remove = [](auto& vect, typename std::remove_reference_t<decltype(vect)>::value_type::pointer value) {
		auto it = std::find_if(vect.begin(), vect.end(), [value](const auto& u_ptr) {
			return u_ptr.get() == value;
		});
		if (it != vect.end()) {
			if (std::next(it) != vect.end()) {
				*it = std::move(vect.back());
			}
			vect.pop_back();
		}
	};

	if (effect->is_dynamic()) {
		auto dyn_effect = dynamic_cast<dynamic_effect*>(effect);
		assert(dyn_effect);
		remove(dynamic_buffs, dyn_effect);
	} else {
		remove(fixed_buffs, effect);
	}
}

void creature::true_hit(int damage) noexcept {
	current_health -= damage;
}

void creature::print(sf::RenderWindow& rw) const noexcept {
	sf::Sprite& current_sprite = sprites[static_cast<unsigned>(current_direction)];
	current_sprite.setPosition(hit_box.top_left.x, hit_box.top_left.y);
	current_sprite.setScale(hit_box.width() / static_cast<float>(current_sprite.getTextureRect().width),
			hit_box.height() / static_cast<float>(current_sprite.getTextureRect().height));
	rw.draw(current_sprite);
}

