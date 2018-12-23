#ifndef DUNGEEP_WORLD_OBJECT_HPP
#define DUNGEEP_WORLD_OBJECT_HPP

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

#include "geometry.hpp"

namespace sf {
	class RenderWindow;
}

class world_object {
public:
	template <typename Child, typename... Args>
	static std::enable_if_t<std::is_base_of_v<world_object, Child>, std::unique_ptr<world_object>>
	make_unique(Args&&... args) {
		return std::unique_ptr<world_object>(new Child(std::forward<Args>(args)...));
	}

	constexpr world_object() noexcept = default;
	explicit constexpr world_object(const dungeep::area_f& ar) noexcept : hit_box(ar) {}

	// purely virtual
	world_object(const world_object& other) = delete;
	world_object(world_object&& other) = delete;
	world_object& operator=(const world_object& other) = delete;
	world_object& operator=(world_object&& other) = delete;

	dungeep::area_f hitbox() const noexcept {
		return hit_box;
	}

	void set_hitbox(dungeep::area_f ar) noexcept {
		hit_box = ar;
	}

	virtual void print(sf::RenderWindow&) const noexcept = 0;

	virtual ~world_object() = default;

protected:
	dungeep::area_f hit_box{};
};

#endif //DUNGEEP_WORLD_OBJECT_HPP
