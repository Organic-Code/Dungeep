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

#include "map_tester.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/Color.hpp>
#include <map_tester.hpp>
#include <sstream>
#include <iomanip>
#include <utils.hpp>

#include "resource_manager.hpp"

namespace
{
	constexpr std::string_view VIEWER_WINDOW_NAME = "Map viewer";
	constexpr std::string_view CONFIG_WINDOW_NAME = "Map config";
	constexpr std::string_view VIEWER_CONFIG_WINDOW_NAME = "Map viewer config";
	constexpr std::string_view DEBUG_INFO_WINDOW_NAME = "Map debug info";

	constexpr room_gen_properties DEFAULT_ROOM_PROPERTIES = {{10.f, 0.1f, 0.f, 0.1f, 1u, 1u, 4u},
	                                                         {10.f, 0.1f, 0.f, 0.1f, 1u, 1u, 4u},
	                                                         2.f,
	                                                         0.1f,
	                                                         0.0f,
	                                                         0.1f};

	constexpr hallway_gen_properties DEFAULT_HALL_PROPERTIES = {0.f,
	                                                            10.f,
	                                                            10.f,
	                                                            0.1f,

	                                                            1.5f,
	                                                            0.1f,
	                                                            1u,
	                                                            2u};
} // namespace

map_tester::map_tester() noexcept
  : m_seed(std::random_device()())
  , m_gen_properties()
  , m_hall_properties(DEFAULT_HALL_PROPERTIES)
  , m_map_size({20u, 20u})
  , m_map()
  , m_image()
  , m_texture()
  , m_from_pos()
  , m_lats_pos()
  , m_selected_load_map(0)
  , m_save_map_name()
  , m_show_zoom()
  , m_zoom_region_size(32.f)
  , m_wall_color(sf::Color::Blue)
  , m_empty_space_color(sf::Color::Black)
  , m_hole_color(sf::Color(122, 122, 122))
  , m_walkable_color(sf::Color(107, 77, 61))
  , m_none_color(sf::Color::Red)
{
	m_save_map_name[0] = '\0';
	m_gen_properties.push_back(DEFAULT_ROOM_PROPERTIES);
	updateMap();
}

void map_tester::configureDockspace(ImGuiID dockspace_id) const
{
	ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
	ImGui::DockBuilderAddNode(dockspace_id, ImGui::GetMainViewport()->Size); // Add empty node

	ImGuiID dock_main_id = dockspace_id;
	ImGuiID dock_id_left =
	  ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
	ImGuiID dock_id_down_right =
	  ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, nullptr, &dock_main_id);
	ImGuiID dock_id_down_left = ImGui::DockBuilderSplitNode(
	  dock_id_down_right, ImGuiDir_Left, 0.5f, nullptr, &dock_id_down_right);

	ImGui::DockBuilderDockWindow(VIEWER_WINDOW_NAME.data(), dock_main_id);
	ImGui::DockBuilderDockWindow(CONFIG_WINDOW_NAME.data(), dock_id_left);
	ImGui::DockBuilderDockWindow(VIEWER_CONFIG_WINDOW_NAME.data(), dock_id_down_right);
	ImGui::DockBuilderDockWindow(DEBUG_INFO_WINDOW_NAME.data(), dock_id_down_left);
	ImGui::DockBuilderFinish(dockspace_id);
}

void map_tester::showConfigWindow()
{
	ImGui::Begin(CONFIG_WINDOW_NAME.data());

	std::vector<std::string> map_list = resource_manager.get_map_list();
	ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() - 50 - GImGui->Style.ItemSpacing.x);
	ImGui::Combo(
	  "##load_combo",
	  &m_selected_load_map,
	  [](void* data, int idx, const char** out_text) -> bool {
		  std::vector<std::string>* data_map_list = static_cast<std::vector<std::string>*>(data);
		  if(idx >= static_cast<int>(data_map_list->size()))
		  {
			  return false;
		  }
		  *out_text =
		    (*data_map_list)[static_cast<std::vector<std::string>::size_type>(idx)].c_str();
		  return true;
	  },
	  &map_list,
	  static_cast<int>(map_list.size()));
	ImGui::SameLine();
	if(ImGui::Button("Load", ImVec2(50, 0)) && m_selected_load_map >= 0
	   && m_selected_load_map < static_cast<int>(map_list.size()))
	{
		std::tie(m_map_size, m_gen_properties, m_hall_properties) = resource_manager.get_map(
		  map_list[static_cast<std::vector<std::string>::size_type>(m_selected_load_map)]);
	}

	ImGui::InputText("##save_txt", m_save_map_name.data(), m_save_map_name.size());
	ImGui::SameLine();
	if(ImGui::Button("Save", ImVec2(50, 0)) && m_save_map_name[0] != '\0')
	{
		resource_manager.save_map(
		  m_save_map_name.data(), m_map_size, m_gen_properties, m_hall_properties);
	}
	ImGui::PopItemWidth();
	ImGui::Separator();

	int tmp = static_cast<int>(m_seed);
	ImGui::InputInt("Seed", &tmp);
	m_seed = static_cast<unsigned int>(tmp);
	tmp = static_cast<int>(m_map_size.width);
	ImGui::InputInt("Map width", &tmp);
	m_map_size.width = static_cast<unsigned int>(tmp);
	tmp = static_cast<int>(m_map_size.height);
	ImGui::InputInt("Map height", &tmp);
	m_map_size.height = static_cast<unsigned int>(tmp);

	if(ImGui::CollapsingHeader("Hallways generation properties", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushItemWidth(-14.0f * ImGui::GetFontSize());
		ImGui::SliderAngle("curliness", &m_hall_properties.curliness, -90.f, 90.0f);
		ImGui::SliderFloat("curly min distance", &m_hall_properties.curly_min_distance, 0.f, 20.f);
		ImGui::SliderFloat(
		  "curly segment avg size", &m_hall_properties.curly_segment_avg_size, 0.f, 20.f);
		ImGui::SliderFloat(
		  "curly segment size dev", &m_hall_properties.curly_segment_size_dev, 0.f, 20.f);
		ImGui::SliderFloat("average width", &m_hall_properties.avg_width, 0.f, 20.f);
		ImGui::SliderFloat("width dev", &m_hall_properties.width_dev, 0.f, 20.f);
		tmp = static_cast<int>(m_hall_properties.min_width);
		ImGui::InputInt("Min width", &tmp);
		m_hall_properties.min_width = static_cast<unsigned int>(tmp);
		tmp = static_cast<int>(m_hall_properties.max_width);
		ImGui::InputInt("Max width", &tmp);
		m_hall_properties.max_width = static_cast<unsigned int>(tmp);
		ImGui::PopItemWidth();
	}

	int properties_number = 0;
	for(room_gen_properties& properties: m_gen_properties)
	{
		const std::string properties_name =
		  std::string("Room generation properties ") + std::to_string(++properties_number);
		ImGui::PushID(properties_number);
		if(ImGui::CollapsingHeader(properties_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if(ImGui::TreeNodeEx("Room properties", ImGuiTreeNodeFlags_DefaultOpen))
			{
				showGenPropertiesConfig(properties.rooms_properties);
				ImGui::TreePop();
			}
			if(ImGui::TreeNodeEx("Holes properties", ImGuiTreeNodeFlags_DefaultOpen))
			{
				showGenPropertiesConfig(properties.holes_properties);
				ImGui::TreePop();
			}
			if(ImGui::TreeNodeEx("Rooms and holes number", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SliderFloat("average rooms number", &properties.avg_rooms_n, 5.f, 100.f);
				ImGui::SliderFloat("rooms number deviation", &properties.rooms_n_dev, 5.f, 100.f);
				ImGui::SliderFloat("average holes number", &properties.avg_holes_n, 5.f, 100.f);
				ImGui::SliderFloat("holes number deviation", &properties.holes_n_dev, 5.f, 100.f);
				ImGui::TreePop();
			}
		}
		ImGui::PopID();
	}

	if(m_gen_properties.size() > 1)
	{
		ImGui::PushStyleColor(ImGuiCol_Button,
		                      static_cast<ImVec4>(ImColor::HSV(1.0f / 7.0f, 0.6f, 0.6f)));
		if(ImGui::Button("-", ImVec2(ImGui::GetWindowContentRegionWidth(), 20)))
		{
			m_gen_properties.pop_back();
		}
		ImGui::PopStyleColor();
	}
	ImGui::PushStyleColor(ImGuiCol_Button,
	                      static_cast<ImVec4>(ImColor::HSV(2.0f / 7.0f, 0.6f, 0.6f)));
	if(ImGui::Button("+", ImVec2(ImGui::GetWindowContentRegionWidth(), 20)))
	{
		m_gen_properties.push_back(DEFAULT_ROOM_PROPERTIES);
	}
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Button,
	                      static_cast<ImVec4>(ImColor::HSV(3.0f / 7.0f, 0.6f, 0.6f)));
	ImGui::Separator();
	if(ImGui::Button("Update", ImVec2(ImGui::GetWindowContentRegionWidth() / 2, 30)))
	{
		updateMap();
	}
	ImGui::SameLine(0, 0);
	ImGui::PushStyleColor(ImGuiCol_Button,
	                      static_cast<ImVec4>(ImColor::HSV(6.0f / 7.0f, 0.6f, 0.6f)));
	if(ImGui::Button("Random", ImVec2(ImGui::GetWindowContentRegionWidth() / 2, 30)))
	{
		m_seed = std::random_device()();
		updateMap();
	}
	ImGui::PopStyleColor(2);

	ImGui::End();
}

void map_tester::showViewerWindow()
{
	ImGui::Begin(VIEWER_WINDOW_NAME.data());
	ImVec2 base_size(static_cast<float>(m_texture.getSize().x),
	                 static_cast<float>(m_texture.getSize().y));
	ImVec2 displayed_size(ImGui::GetWindowContentRegionWidth(),
	                      base_size.y / base_size.x * ImGui::GetWindowContentRegionWidth());
	ImGui::Image(m_texture, displayed_size);
	if(ImGui::IsItemHovered())
	{
		ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::BeginTooltip();
		dungeep::point_i pos;
		pos.x = static_cast<int>((io.MousePos.x - cursor_pos.x) * base_size.x / displayed_size.x);
		if(pos.x < 0)
		{
			pos.x = 0;
		}
		pos.y = static_cast<int>(base_size.y
		                         + (io.MousePos.y - cursor_pos.y) * base_size.y / displayed_size.y);
		if(pos.y < 0.)
		{
			pos.y = 0;
		}

		if(pos != m_lats_pos)
		{
			m_image.setPixel(
			  static_cast<unsigned int>(m_lats_pos.x),
			  static_cast<unsigned int>(m_lats_pos.y),
			  tileColor(
			    m_map[static_cast<size_t>(m_lats_pos.x)][static_cast<size_t>(m_lats_pos.y)]));
			m_image.setPixel(static_cast<unsigned int>(pos.x),
			                 static_cast<unsigned int>(pos.y),
			                 tileColor(tiles::none));
			m_texture.loadFromImage(m_image);
			m_lats_pos = pos;
		}

		ImGui::Text("From: (%d, %d)\nTo: (%d, %d)", m_from_pos.x, m_from_pos.y, pos.x, pos.y);
		if(m_show_zoom)
		{
			sf::FloatRect zone(
			  std::max(0.f,
			           std::min(static_cast<float>(pos.x) - ((m_zoom_region_size - 1) / 2),
			                    base_size.x - (m_zoom_region_size - 1))),
			  std::max(0.f,
			           std::min(static_cast<float>(pos.y) - ((m_zoom_region_size - 1) / 2),
			                    base_size.y - (m_zoom_region_size - 1))),
			  (m_zoom_region_size),
			  (m_zoom_region_size));
			ImGui::Image(m_texture, ImVec2(128.f, 128.f), zone);
		}
		ImGui::EndTooltip();

		if(ImGui::IsMouseClicked(0))
		{
			m_from_pos = pos;
		}
		if(ImGui::IsMouseClicked(1))
		{
			std::vector<dungeep::point_i> path =
			  m_map.path_to_pt(m_from_pos, pos, std::numeric_limits<float>::infinity());
			updateMapView(); // clear last path
			for(const dungeep::point_i& pt: path)
			{
				m_image.setPixel(
				  static_cast<unsigned int>(pt.x), static_cast<unsigned int>(pt.y), m_none_color);
			}
			m_texture.loadFromImage(m_image);
		}
	}
	ImGui::End();
}

void map_tester::showViewerConfigWindow()
{
	ImGui::Begin(VIEWER_CONFIG_WINDOW_NAME.data());
	ImGui::Checkbox("Show zoom", &m_show_zoom);
	ImGui::SliderFloat("Zoom region", &m_zoom_region_size, 4.f, 64.f);

	bool changed = false;
	changed |= showColorConfig("Wall", m_wall_color);
	changed |= showColorConfig("Empty space", m_empty_space_color);
	changed |= showColorConfig("Hole", m_hole_color);
	changed |= showColorConfig("Walkable", m_walkable_color);
	changed |= showColorConfig("None", m_none_color);
	changed |= ImGui::Button("Clear path");
	if(changed)
	{
		updateMapView();
	}
	ImGui::End();
}

void map_tester::updateMap()
{
	dungeep::random_engine.seed(m_seed);
	m_map.generate(m_map_size, m_gen_properties, m_hall_properties);
	updateMapView();
}

void map_tester::updateMapView()
{
	m_image.create(m_map_size.width, m_map_size.height);
	for(unsigned int i = 0; i < m_map_size.width; ++i)
	{
		for(unsigned int j = 0; j < m_map_size.height; ++j)
		{
			m_image.setPixel(i, j, tileColor(m_map[i][j]));
		}
	}
	m_texture.loadFromImage(m_image);
}

void map_tester::showGenPropertiesConfig(zone_gen_properties& properties)
{
	ImGui::PushItemWidth(-14.0f * ImGui::GetFontSize());
	ImGui::SliderFloat("average size", &properties.avg_size, 5.f, 500.f);
	ImGui::SliderFloat("size deviation", &properties.size_deviation, 0.f, 50.f);
	ImGui::SliderFloat("borders fuzziness", &properties.borders_fuzzinness, 0.f, 50.f);
	ImGui::SliderFloat("borders fuzzy deviation", &properties.borders_fuzzy_deviation, 0.f, 50.f);

	int tmp = static_cast<int>(properties.borders_fuzzy_distance);
	ImGui::SliderInt("borders fuzzy distance", &tmp, 0, 50);
	properties.borders_fuzzy_distance = static_cast<unsigned int>(tmp);

	tmp = static_cast<int>(properties.min_height);
	ImGui::SliderInt("minimum height", &tmp, 0, 50);
	properties.min_height = static_cast<unsigned int>(tmp);

	tmp = static_cast<int>(properties.max_height);
	ImGui::SliderInt("maximum height", &tmp, 0, 50);
	properties.max_height = static_cast<unsigned int>(tmp);
}

bool map_tester::showColorConfig(const std::string_view label, sf::Color& color)
{
	std::array<float, 3> color_array{static_cast<float>(color.r) / 255.0f,
	                                 static_cast<float>(color.g) / 255.0f,
	                                 static_cast<float>(color.b) / 255.0f};
	const bool changed =
	  ImGui::ColorEdit3(label.data(), color_array.data(), ImGuiColorEditFlags_NoInputs);
	color.r = static_cast<sf::Uint8>(color_array[0] * 255.0f);
	color.g = static_cast<sf::Uint8>(color_array[1] * 255.0f);
	color.b = static_cast<sf::Uint8>(color_array[2] * 255.0f);
	return changed;
}

void map_tester::showDebugInfoWindow()
{
	ImGui::Begin(DEBUG_INFO_WINDOW_NAME.data());
	if(ImGui::TreeNodeEx("Times", ImGuiTreeNodeFlags_DefaultOpen))
	{
		auto print = [](std::string_view txt, std::chrono::milliseconds time) {
			std::stringstream stringstream;
			stringstream << txt << ": " << std::fixed << std::setprecision(3) << std::setw(6)
			             << std::setfill(' ') << static_cast<float>(time.count()) / 1000.f;
			ImGui::TextUnformatted(stringstream.str().c_str());
		};
		print("Total generation time", m_map.total_generation_time);
		print("Halls generation time", m_map.halls_generation_time);
		print("Rooms generation time", m_map.rooms_generation_time);
		print("Fuzzy generation time", m_map.fuzzy_generation_time);
		ImGui::TreePop();
	}
	if(ImGui::TreeNodeEx("Counts", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Expected room count: %u", m_map.expected_room_count);
		ImGui::Text("Actual room count:   %u", m_map.expected_hole_count);
		ImGui::Text("Expected hole count: %u", m_map.actual_room_count);
		ImGui::Text("Actual hole count:   %u", m_map.actual_hole_count);
		ImGui::TreePop();
	}
	ImGui::End();
}

const sf::Color& map_tester::tileColor(const tiles& tile) const
{
	switch(tile)
	{
		case tiles::wall:
			return m_wall_color;
		case tiles::empty_space:
			return m_empty_space_color;
		case tiles::hole:
			return m_hole_color;
		case tiles::walkable:
			return m_walkable_color;
		case tiles::none:
			[[fallthrough]];
		default:
			return m_none_color;
	}
}
