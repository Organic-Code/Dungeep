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

constexpr dungeep::direction dungeep::operator-(direction d) noexcept {
	switch(d) {
		case direction::top:
			return direction::bot;
		case direction::bot:
			return direction::top;
		case direction::left:
			return direction::right;
		case direction::right:
			return direction::left;
		case direction::top_left:
			return direction::bot_right;
		case direction::top_right:
			return direction::bot_left;
		case direction::bot_left:
			return direction::top_right;
		case direction::bot_right:
			return direction::top_left;
		case direction::none:
			return direction::none;
	}
	return direction::none;
}

template <typename T>
constexpr dungeep::point<T> dungeep::point<T>::operator+(const point& p) const noexcept {
	point tmp(*this);
	tmp += p;
	return tmp;
}


template <typename T>
constexpr dungeep::point<T>& dungeep::point<T>::operator+=(const point& p) noexcept {
	x += p.x;
	y += p.y;
	return *this;
}

template <typename T>
constexpr dungeep::point<T> dungeep::point<T>::operator-(const point& p) const noexcept {
	point tmp(*this);
	tmp -= p;
	return tmp;
}

template <typename T>
constexpr dungeep::point<T>& dungeep::point<T>::operator-=(const point& p) noexcept {
	x -= p.x;
	y -= p.y;
	return *this;
}

template <typename T>
constexpr dungeep::point<T> dungeep::point<T>::operator/(T val) const noexcept {
	point tmp(*this);
	tmp /= val;
	return tmp;
}

template <typename T>
constexpr dungeep::point<T>& dungeep::point<T>::operator/=(T val) noexcept {
	x /= val;
	y /= val;
	return *this;
}

template <typename T>
constexpr dungeep::point<T> dungeep::point<T>::operator*(T val) const noexcept {
	point tmp(*this);
	tmp *= val;
	return tmp;
}

template <typename T>
constexpr dungeep::point<T>& dungeep::point<T>::operator*=(T val) noexcept {
	x *= val;
	y *= val;
	return *this;
}

template <typename T>
constexpr bool dungeep::point<T>::is_in(const area<T>& ar) const noexcept {
	ar.assert_well_formed();
	return ar.top_left.x  <= this->x && ar.top_left.y  <= this->y
	       && ar.bot_right.x >= this->x && ar.bot_right.y >= this->y;
}

template<typename T>
void dungeep::point<T>::translate(direction d, T distance) noexcept {
	auto diag_dist = static_cast<T>(std::hypot(distance, distance));
	switch(d) {
		case direction::top:
			this->y -= distance;
			break;
		case direction::bot:
			this->y += distance;
			break;
		case direction::left:
			this->x -= distance;
			break;
		case direction::right:
			this->x += distance;
			break;
		case direction::top_left:
			this->x -= diag_dist;
			this->y -= diag_dist;
			break;
		case direction::top_right:
			this->x += diag_dist;
			this->y -= diag_dist;
			break;
		case direction::bot_left:
			this->x -= diag_dist;
			this->y += diag_dist;
			break;
		case direction::bot_right:
			this->x += diag_dist;
			this->y += diag_dist;
			break;
		case direction::none:
			break;
	}
}

template<typename T>
void dungeep::point<T>::translate_fixed(dungeep::direction d, T distance) noexcept {
	switch(d) {
		case direction::top:
			this->y -= distance;
			break;
		case direction::bot:
			this->y += distance;
			break;
		case direction::left:
			this->x -= distance;
			break;
		case direction::right:
			this->x += distance;
			break;
		case direction::top_left:
			this->x -= distance;
			this->y -= distance;
			break;
		case direction::top_right:
			this->x += distance;
			this->y -= distance;
			break;
		case direction::bot_left:
			this->x -= distance;
			this->y += distance;
			break;
		case direction::bot_right:
			this->x += distance;
			this->y += distance;
			break;
		case direction::none:
			break;
	}

}

template<typename T>
constexpr bool dungeep::point<T>::operator==(const point& p) const noexcept {
	static_assert(std::numeric_limits<T>::is_integer, "Values are compared with '=='");
	return p.x == x && y == p.y;
}

template<typename T>
constexpr bool dungeep::point<T>::operator!=(const point& p) const noexcept {
	return !(*this == p);
}

template<typename T>
template<typename U>
void dungeep::point<T>::rotate(U angle) {
	auto cos = std::cos(angle);
	auto sin = std::sin(angle);

	using trigo_t = decltype(cos);

	T new_x = static_cast<T>(static_cast<trigo_t>(x) * cos - static_cast<trigo_t>(y) * sin);
	y = static_cast<T>(static_cast<trigo_t>(x) * sin + static_cast<trigo_t>(y) * cos);
	x = new_x;
}

template<typename T>
template<typename U>
void dungeep::point<T>::scale_to(U new_size) {
	auto scale_factor = new_size / static_cast<U>(length());
	x = static_cast<T>(static_cast<U>(x) * scale_factor);
	y = static_cast<T>(static_cast<U>(y) * scale_factor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr bool dungeep::area<T>::contains(const area& other) const noexcept {
	this->assert_well_formed();
	other.assert_well_formed();
	return other.bot_right.is_in(*this) && other.top_left.is_in(*this);
}

template <typename T>
constexpr bool dungeep::area<T>::is_in(const area& other) const noexcept {
	return other.contains(*this);
}

template <typename T>
constexpr bool dungeep::area<T>::collides_with(const area& other) const noexcept {
	this->assert_well_formed();
	other.assert_well_formed();
	return other.bot_right.is_in(*this) || other.top_left.is_in(*this) || this->bot_right.is_in(other);
}

template <typename T>
constexpr T dungeep::area<T>::width() const noexcept {
	assert_well_formed();
	return bot_right.x - top_left.x;
}

template <typename T>
constexpr T dungeep::area<T>::height() const noexcept {
	assert_well_formed();
	return bot_right.y - top_left.y;
}

template <typename T>
constexpr T dungeep::area<T>::size() const noexcept {
	assert_well_formed();

	return width() * height();

}

template <typename T>
constexpr void dungeep::area<T>::assert_well_formed() const noexcept {
	assert(top_left.x <= bot_right.x && top_left.y <= bot_right.y);
}
