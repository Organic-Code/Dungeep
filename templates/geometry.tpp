///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018, Lucas Lazare                                                                                                     ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  		files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,  ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  		is furnished to do so, subject to the following conditions:                                                                 ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  		warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or       ///
///  copyright holders X be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,      ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <cassert>

inline bool dungeep::point::is_in(const area& ar) const noexcept {
	ar.assert_well_formed();
	return ar.top_left.x  <= this->x && ar.top_left.y  <= this->y
	       && ar.bot_right.x >= this->x && ar.bot_right.y >= this->y;
}

inline float dungeep::point::length() const noexcept {
	return std::hypot(x,y);
}

inline bool dungeep::area::contains(const area& other) const noexcept {
	this->assert_well_formed();
	other.assert_well_formed();
	return other.bot_right.is_in(*this) && other.top_left.is_in(*this);
}

inline bool dungeep::area::collides_with(const area& other) const noexcept {
	this->assert_well_formed();
	other.assert_well_formed();
	return other.bot_right.is_in(*this) || other.top_left.is_in(*this) || this->bot_right.is_in(other);
}

inline float dungeep::area::size() const noexcept {
	point hz{bot_right.x - top_left.x, 0};
	point vt{bot_right.y - top_left.y, 0};

	return hz.length() * vt.length();

}

inline void dungeep::area::assert_well_formed() const noexcept {
	assert(top_left.x <= bot_right.x && top_left.y <= bot_right.y);
}
