#ifndef DUNGEEP_MAP_TESTER_HPP
#define DUNGEEP_MAP_TESTER_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018, Maxime Pinard                                                                                                    ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,         ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  is furnished to do so, subject to the following conditions:                                                                        ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,        ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "environment/map.hpp"

#include <imgui.h>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <random>
#include <memory>
#include <utils/resource_manager.hpp>

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

	const sf::Color& tileColor(const tiles& tile) const;

	unsigned int m_seed;
	resources::map_info m_map_props;
	map m_map;

	sf::Image m_image;
	sf::Texture m_texture;
	dungeep::point_i m_from_pos;
	dungeep::point_i m_lats_pos;

	int m_selected_load_map;
	std::array<char, 2048> m_save_map_name;

	bool m_show_zoom;
	float m_zoom_region_size;
	sf::Color m_wall_color;
	sf::Color m_empty_space_color;
	sf::Color m_hole_color;
	sf::Color m_walkable_color;
	sf::Color m_none_color;
};

#endif //DUNGEEP_MAP_TESTER_HPP
