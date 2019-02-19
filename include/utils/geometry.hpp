#ifndef DUNGEEP_AREA_HPP
#define DUNGEEP_AREA_HPP

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
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,        ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <cstdint>

namespace dungeep {

template <typename>
struct area;

enum class direction {
	top,
	bot,
	left,
	right,
	top_left,
	top_right,
	bot_left,
	bot_right,
	none
};

constexpr direction operator-(direction d) noexcept;

template <typename T>
struct point {

	static_assert(std::is_fundamental_v<T>);

	constexpr point() noexcept : x(0), y(0) {}
	constexpr point(T x_, T y_) noexcept : x(x_), y(y_) {}
	constexpr point(const point& o) noexcept : x(o.x), y(o.y) {}

	template <typename U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
	explicit constexpr point(const point<U>& o) noexcept : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)) {}

	constexpr point& operator=(const point&) noexcept = default;

	T x, y;

	constexpr point operator+(const point& p) const noexcept;

	constexpr point& operator+=(const point& p) noexcept;

	constexpr point operator-(const point& p) const noexcept;

	constexpr point& operator-=(const point& p) noexcept;

	constexpr point operator/(T val) const noexcept;

	constexpr point& operator/=(T val) noexcept;

	constexpr point operator*(T val) const noexcept;

	constexpr point& operator*=(T val) noexcept;

	constexpr bool operator==(const point& p) const noexcept;

	constexpr bool operator!=(const point& p) const noexcept;

	constexpr bool is_in(const area<T>&) const noexcept;

	template <typename U>
	void rotate(U angle);

	template <typename U>
	void scale_to(U new_size);

	void translate(direction d, T distance) noexcept;

	void translate_fixed(direction d, T distance) noexcept;

	auto length() const noexcept {
		return std::hypot(x, y);
	}

};

template <typename T>
constexpr point<T> operator*(T val, const point<T>& p) {
	return p * val;
}

template <typename T>
struct area {
	static const area<T> null;

	constexpr area() noexcept : top_left(), bot_right() {}
	constexpr area(const point<T>& tl, const point<T>& br) noexcept : top_left(tl), bot_right(br) {}
	constexpr area(const area&) noexcept = default;
	constexpr area& operator=(const area&) noexcept = default;

	point<T> top_left; // smallest x & y
	point<T> bot_right; // greatest x & y

	constexpr T width() const noexcept;

	constexpr T height() const noexcept;

	constexpr T size() const noexcept;

	constexpr bool contains(const area& other) const noexcept;

	constexpr bool is_in(const area& other) const noexcept;

	constexpr bool collides_with(const area& other) const noexcept;

	constexpr void assert_well_formed() const noexcept;

	constexpr point<T> center() const noexcept;
};

template <typename T>
const area<T> area<T>::null = {{T{}, T{}}, {T{}, T{}}};

using point_ui = point<unsigned int>;
using area_ui = area<unsigned int>;

using point_i = point<int>;
using area_i = area<int>;

using point_f = point<float>;
using area_f = area<float>;

using dim_uc = point<std::uint8_t>;
using dim_ui = point_ui;
using dim_f = point_f;
}

#include "geometry.tpp"

#endif //DUNGEEP_AREA_HPP
