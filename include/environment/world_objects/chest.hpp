#ifndef DUNGEEP_CHEST_HPP
#define DUNGEEP_CHEST_HPP

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

#include <optional>

#include "world_object.hpp"
#include "item.hpp"
#include "iconned/fixed.hpp"
#include "environment/world_proxy.hpp"

enum class chest_level {
	// TODO: utiliser une clef pour ouvrir un coffre ne "consomme" la clef que 20% du temps, chance de casser augmentant avec le nombre de coffres ouverts
	// Compétence passive : ça se casse moi souvent // le taux d'usure est plus faible
	// Tabasser un coffre peut permettre de l'ouvrir, mais a une chance d'user l'arme et de baisser ses dégats
			wood,
	bronze,
	silver,
	gold,
	diamond,
};

class chest : public world_object {
public:

	chest(chest_level l) : level(l), ite{item::generate_rand(l)} {}

	void drop(world_proxy& proxy) noexcept {
		if (ite) {
			proxy.create_entity(std::move(ite));
			proxy.delete_entity(this);
		}
	}


	void print(sf::RenderWindow&) const noexcept override;

private:
	chest_level level;
	std::unique_ptr<item> ite;
};

#endif //DUNGEEP_CHEST_HPP
