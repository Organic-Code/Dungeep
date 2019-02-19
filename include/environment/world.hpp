#ifndef DUNGEEP_WORLD_HPP
#define DUNGEEP_WORLD_HPP

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

#include <random>

#include "environment/world_objects/dynamic_object.hpp"
#include "utils/quadtree.hpp"
#include "map.hpp"

class world {
	friend class world_proxy;
	// TODO pièges ?

public:
	world() = default;

	void generate_next_level();

	void seed_world(std::mt19937_64::result_type seed) {
		shared_random.seed(seed);
		std::seed_seq seq{{
				              local_random()
				              , std::random_device()() + 0ul
				              , seed
			                  , local_random()
				              , shared_random()
				              , std::random_device()() + 0ul
		              }};
		local_random.seed(seq);
	}

	void next_tick();

private:

	unsigned int mobs_per_100_tiles() const noexcept {
		return 2 * current_level + 10;
	}

	std::mt19937_64 shared_random{}; // NOLINT
	std::mt19937_64 local_random{std::random_device{}()};
	dungeep::quadtree<dungeep::qtree_unique_ptr<dynamic_object>> dynamic_objects{dungeep::area_f::null};
	dungeep::quadtree<dungeep::qtree_unique_ptr<world_object>> static_objects{dungeep::area_f::null};

	unsigned int current_level{0u};

	map shared_map{};

};

#endif //DUNGEEP_WORLD_HPP
