
#include <world_objects/player.hpp>

#include "world_objects/player.hpp"

int player::sleep() noexcept {
	return 0;
}

void player::tick(world_proxy& world) noexcept {
	// TODO
}

void player::true_hit(int damage) noexcept {
	creature::true_hit(damage);
}

void player::move(dungeep::area_f) noexcept {
	// TODO
}

bool player::is_local() const noexcept {
	// TODO
	return false;
}

void player::drop_item(unsigned int item_index) noexcept {
	// TODO
}

void player::lose_item(unsigned int item_index) noexcept {
	// TODO
}

void player::gain_item(std::unique_ptr<fixed_effect>&&) noexcept {
	// TODO
}

void player::gain_gold(unsigned int gold) noexcept {
	// TODO
}

void player::lose_gold(unsigned int gold) noexcept {
	// TODO
}

unsigned int player::get_current_gold() const noexcept {
	// TODO
	return 0;
}

void player::set_armor(int) noexcept {
	// TODO
}

void player::set_resist(int) noexcept {
	// TODO
}

void player::set_max_hp(int) noexcept {
	// TODO
}

void player::set_hp(int) noexcept {
	// TODO
}

int player::get_mana() const noexcept {
	return 0;
}
