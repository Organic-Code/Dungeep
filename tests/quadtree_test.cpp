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
#include <catch2/catch.hpp>
#include <quadtree.hpp>
#include <geometry.hpp>

using area = dungeep::area<float>;
using point = dungeep::point<float>;
using dungeep::quadtree;

namespace {

	struct collider {

		collider(const collider&) noexcept = default;
		collider& operator=(const collider&) noexcept = default;
		bool operator==(const collider& other) const {
			// lazy equality test
			return hitbox_.contains(other.hitbox_) && other.hitbox_.contains(hitbox_);
		}

		const area& hitbox() const noexcept {
			return hitbox_;
		}

		void set_hitbox(const area& ar) noexcept {
			hitbox_ = ar;
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

	SECTION("Container") {
		collider c1{{{24.f, 24.f}, {26.f, 26.f}}};
		collider c2{{{24.f, 74.f}, {26.f, 76.f}}};
		collider c3{{{74.f, 74.f}, {76.f, 76.f}}};
		collider c4{{{74.f, 24.f}, {76.f, 26.f}}};
		collider c5{{{20.f, 20.f}, {30.f, 80.f}}};

		qt.insert(c1);
		qt.insert(c2);
		qt.insert(c3);
		qt.insert(c4);
		qt.insert(c5);

		REQUIRE(qt.size() == 5);

		SECTION("Modifying & visiting") {
			auto it = qt.find(c1);
			REQUIRE(it != qt.end());

			collider c = qt.extract(it);
			CHECK(qt.size() == 4);

			CHECK(c == c1);

			it = qt.find(c2);
			REQUIRE(it != qt.end());

			qt.move(it, c.hitbox());

			qt.move(c4, {{25.f, 25.5f}, {25.f, 25.5f}});
			it = qt.insert(c3);

			int hits = 0;
			qt.visit(c.hitbox(), [&hits, &qt](decltype(qt)::iterator it_) {
				++hits;
				static bool b = true;
				return std::exchange(b, false);
			});
			CHECK(hits == 3);
			CHECK(qt.size() == 4);
			it = qt.find(c3);
			qt.erase(it);
			CHECK(qt.size() == 3);
			CHECK(qt.has_collision(c.hitbox()));
			CHECK(!qt.has_collision({{0.f, 0.f}, {2.f, 2.f}}));
		}
	}
}
