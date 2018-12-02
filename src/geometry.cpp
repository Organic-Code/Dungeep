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

#include "geometry.hpp"

point point::operator+(const point& p) const noexcept {
	point tmp(*this);
	tmp += p;
	return tmp;
}

point& point::operator+=(const point& p) noexcept {
	x += p.x;
	y += p.y;
	return *this;
}

point point::operator-(const point& p) const noexcept {
	point tmp(*this);
	tmp -= p;
	return tmp;
}

point& point::operator-=(const point& p) noexcept {
	x -= p.x;
	y -= p.y;
	return *this;
}

point point::operator/(float val) const noexcept {
	point tmp(*this);
	tmp /= val;
	return tmp;
}

point& point::operator/=(float val) noexcept {
	x /= val;
	y /= val;
	return *this;
}

point point::operator*(float val) const noexcept {
	point tmp(*this);
	tmp *= val;
	return tmp;
}

point& point::operator*=(float val) noexcept {
	x *= val;
	y *= val;
	return *this;
}
