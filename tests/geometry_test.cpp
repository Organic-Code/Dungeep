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

#include <catch2/catch.hpp>
#include <geometry.hpp>

SCENARIO("Geometry has collision and sizes") {
	using point = dungeep::point<float>;
	using area = dungeep::area<float>;

	const point p1{0.f, 0.f};
	const point p2{10.f, 10.f};
	const point p3{-10.f, -10.f};
	const point p4{2.f, -5.f};
	const point p5{-4.f, 1.f};

	const area a1{{  0.f,   0.f}, {100.f, 100.f}};
	const area a2{{-10.f, -15.f}, {  3.f,   7.f}};
	const area a3{{  2.f,   9.f}, {  3.f,  10.f}};
	const area a4{{  5.f,   5.f}, {  5.f,   5.f}};
	const area a5{{  2.f,   2.f}, { 10.f,  10.f}};

	GIVEN("Points") {
		CHECK(p1.length() == 0.f);
		CHECK(p2.length() == Approx(14.142f));
		CHECK(p3.length() == Approx(14.142f));
		CHECK(p4.length() == Approx(5.3852f));
		CHECK(p5.length() == Approx(4.1231f));
	}

	GIVEN("Areas") {
		CHECK(a1.size() == Approx(10000.f));
		CHECK(a2.size() == Approx(286.f));
		CHECK(a3.size() == Approx(1.f));
		CHECK(a4.size() == Approx(0.f));
		CHECK(a5.size() == Approx(64.f));
	}

	GIVEN("Points and areas") {
		CHECK(p1.is_in(a2));
		CHECK(!p1.is_in(a3));
		CHECK(p2.is_in(a1));
		CHECK(!p2.is_in(a3));
		CHECK(p2.is_in(a5));
		CHECK(p4.is_in(a2));
		CHECK(!p4.is_in(a4));
		CHECK(p5.is_in(a2));
		CHECK(!p5.is_in(a5));
	}

	GIVEN("Two different areas") {
		CHECK(a1.collides_with(a1));
		CHECK(a2.collides_with(a2));
		CHECK(a3.contains(a3));
		CHECK(a4.collides_with(a4));
		CHECK(a4.contains(a4));

		CHECK(a1.collides_with(a2));
		CHECK(a1.collides_with(a3));
		CHECK(a1.collides_with(a4));
		CHECK(a1.collides_with(a5));

		CHECK(!a1.contains(a2));
		CHECK(a1.contains(a3));
		CHECK(a1.contains(a4));
		CHECK(a1.contains(a5));

		CHECK(a2.collides_with(a1));
		CHECK(!a2.collides_with(a3));
		CHECK(!a2.collides_with(a4));
		CHECK(a2.collides_with(a5));

		CHECK(!a2.contains(a1));
		CHECK(!a2.contains(a3));
		CHECK(!a2.contains(a4));
		CHECK(!a2.contains(a5));

		CHECK(a4.collides_with(a1));
		CHECK(!a4.collides_with(a2));
		CHECK(!a4.collides_with(a3));
		CHECK(a4.collides_with(a5));

		CHECK(!a4.contains(a1));
		CHECK(!a4.contains(a2));
		CHECK(!a4.contains(a3));
		CHECK(!a4.contains(a5));
	}
}