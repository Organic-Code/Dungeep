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

#include "utils/geometry.hpp"

namespace sf {
	class RenderWindow;
}
class player;

class world_object {
public:
	// purely virtual
	world_object(const world_object& other) = delete;
	world_object(world_object&& other) = delete;
	world_object& operator=(const world_object& other) = delete;
	world_object& operator=(world_object&& other) = delete;

	constexpr world_object() noexcept = default;
	explicit constexpr world_object(const dungeep::area_f& ar) noexcept : hit_box(ar) {}

	dungeep::area_f hitbox() const noexcept {
		return hit_box;
	}

	void set_hitbox(const dungeep::area_f& ar) noexcept {
		hit_box = ar;
	}

	// TODO: use for spell/fervor/potion/object/... ?
	dungeep::area_f get_resized(float width_percent) noexcept {
		return get_resized(width_percent, width_percent);
	}

	dungeep::area_f get_resized(float width_percent, float height_percent) const noexcept {
		using dungeep::point_f;
		auto hit_box_cpy = hit_box;

		point_f diff = hit_box_cpy.top_left - hit_box_cpy.bot_right;
		point_f new_diff = diff;
		new_diff.x *= width_percent;
		new_diff.y *= height_percent;

		point_f translate = (diff - new_diff) / 2;
		hit_box_cpy.top_left -= translate;
		hit_box_cpy.bot_right += translate;

		return hit_box_cpy;
	}

	virtual void print(sf::RenderWindow&) const noexcept = 0;

	virtual void interact_with(player&) noexcept = 0;

	virtual ~world_object() = default;

protected:
	dungeep::area_f hit_box{};
};

#endif //DUNGEEP_WORLD_OBJECT_HPP
