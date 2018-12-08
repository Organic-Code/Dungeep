#include <iostream>
#include "map.hpp"
#include "quadtree.hpp"

int main() {
	std::mt19937_64 mt{std::random_device()()};

	std::vector<room_gen_properties> v;
	v.push_back(room_gen_properties{
			{
				100.f,
				3.f,
				2.f,
				1.f,
				4u,
				5u,
				20u
			},
			{
				20.f,
				2.f,
				1.f,
				0.1f,
				1u,
				1u,
				1u
			},
			10.5f,
			2.f,
			2.2f,
			0.5f
	});

	map m({150, 30}, v, mt);
	std::cout << m;
	return 0;
}