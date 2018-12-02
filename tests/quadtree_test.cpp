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

#include <cstdlib>
#include <ctime>
#include <catch.hpp>
#include <quadtree.hpp>
#include <geometry.hpp>

using namespace dungeep;

namespace {

	struct collider {

		collider(const collider&) noexcept = default;
		collider& operator=(const collider&) noexcept = default;

		const area& hitbox() const noexcept {
			return hitbox_;
		}

		area hitbox_;
	};

	point rand_point() {
		return {(rand() % 9900) / 100.f, (rand() % 9900) / 100.f};
	}

	area rand_area() {
		point pt = rand_point();
		return {pt, pt + point{1.f, 1.f}};
	}
}

TEST_CASE("Quadtree") {
	quadtree<collider> qt{{{0.f, 0.f}, {100.f, 100.f}}, 10, 2};
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	SECTION("Sizes") {
		CHECK(qt.empty());
		for (auto i = 0u ; i < 42 ; ++i) {
			qt.insert({rand_area()});
		}
		CHECK(qt.size() == 42);
		qt.clear();
		CHECK(qt.size() == 0);
		CHECK(qt.empty());

	}
}
