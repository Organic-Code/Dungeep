#ifndef DUNGEEP_WORLD_PROXY_HPP
#define DUNGEEP_WORLD_PROXY_HPP

#include <memory>
#include <vector>
#include <random>

#include "world.hpp"
#include "tiles.hpp"

class world_object;
class creature;

namespace dungeep {
	template <typename>
	struct area;
	using area_f = area<float>;

	template <typename>
	struct point;
	using point_i = point<int>;
}

class world_proxy {
	friend class world;
public:
	explicit world_proxy(world& w) noexcept : tied_world(w) {}

	std::mt19937_64& world_shared_random_engine() {
		return tied_world.shared_random;
	}

	std::mt19937_64::result_type world_shared_rand() {
		return world_shared_random_engine()();
	}

	std::mt19937_64& world_local_random_engine() {
		return tied_world.local_random;
	}

	std::mt19937_64::result_type  world_local_rand() {
		return world_local_random_engine()();
	}

	void create_entity(std::unique_ptr<world_object>&& ptr) {
		new_objects.emplace_back(std::move(ptr));
	}

	void delete_entity(world_object* ptr) {
		deleted_objects.emplace_back(ptr);
	}

	template <typename Pred>
	std::vector<std::unique_ptr<world_object>> find_targets(const dungeep::area_f& area, Pred&& predicate);

	std::vector<dungeep::point_i> find_path(const dungeep::point_i& dep, const dungeep::point_i& arr, int max_depth = 30) const;

	tiles operator()(int x, int y) const;

//	creature_death_lid register_creature_death_listener(const std::function<void(std::unique_ptr<creature>&)>&);
//	void remove_listener(int);
//  ... register ...

private:
	world& tied_world;

	std::vector<std::unique_ptr<world_object>> new_objects{};
	std::vector<world_object*> deleted_objects{};
};

#endif //DUNGEEP_WORLD_PROXY_HPP
