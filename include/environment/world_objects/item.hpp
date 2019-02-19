#ifndef DUNGEEP_DROPPED_ITEM_HPP
#define DUNGEEP_DROPPED_ITEM_HPP

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
#include "world_object.hpp"
#include "chest.hpp"

enum class chest_level;

class item final : public world_object {
public:
	static std::unique_ptr<item> generate_rand(chest_level);

	explicit item(std::unique_ptr<fixed_effect>&& it) noexcept : item_effects(std::move(it)) {}

	void print(sf::RenderWindow& rw) const noexcept override {
		item_effects->print_icon_at(rw, hitbox());
	}

	void interact_with(player&) noexcept override {
		// todo
	}


private:
	std::unique_ptr<fixed_effect> item_effects;

};

#endif //DUNGEEP_DROPPED_ITEM_HPP
