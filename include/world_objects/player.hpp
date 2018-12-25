#ifndef DUNGEEP_PLAYER_HPP
#define DUNGEEP_PLAYER_HPP

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

#include <vector>

#include "world_objects/creature.hpp"
#include "geometry.hpp"

class fixed_effect;
class dynamic_effect;

class player final : public creature {
public:
	void tick(world_proxy& world) noexcept override;

	void print(sf::RenderWindow&) const noexcept override;

	void true_hit(int damage) noexcept override;

	void move(dungeep::area_f) noexcept;

	void drop_item(unsigned int item_index) noexcept;

	void lose_item(unsigned int item_index) noexcept;

	void gain_item(std::unique_ptr<fixed_effect>&&) noexcept;

	void gain_gold(unsigned int gold) noexcept;

	void lose_gold(unsigned int gold) noexcept;

	unsigned int get_current_gold() const noexcept;

	void set_armor(int) noexcept;

	void set_resist(int) noexcept;

	void set_max_hp(int) noexcept;

	void set_hp(int) noexcept;

	int get_mana() const noexcept;

private:

	std::vector<std::unique_ptr<fixed_effect>> fixed_items;
	std::vector<std::unique_ptr<dynamic_effect>> dynamic_items;
};

#endif //DUNGEEP_PLAYER_HPP
