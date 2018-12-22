#ifndef DUNGEEP_MAP_TESTER_HPP
#define DUNGEEP_MAP_TESTER_HPP

#include "map.hpp"

#include <imgui.h>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <random>
#include <memory>

class map_tester final
{

public:
	map_tester() noexcept;

	void configureDockspace(ImGuiID dockspace_id) const;

	void showConfigWindow();

	void showViewerWindow();

	void showViewerConfigWindow();

	void showDebugInfoWindow();

private:
	void updateMap();

	void updateMapView();

	void showGenPropertiesConfig(zone_gen_properties& properties);

	bool showColorConfig(std::string_view label, sf::Color& color);

	unsigned int m_seed;
	std::vector<room_gen_properties> m_gen_properties;
	hallway_gen_properties m_hall_properties;
	map::size_type m_map_size;
	map m_map;

	sf::Image m_image;
	sf::Texture m_texture;
	dungeep::point_i m_from_pos;

	sf::Color wall_color;
	sf::Color empty_space_color;
	sf::Color hole_color;
	sf::Color walkable_color;
	sf::Color none_color;
};

#endif //DUNGEEP_MAP_TESTER_HPP
