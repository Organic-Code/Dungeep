#include "map_tester.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/Color.hpp>

namespace
{
	constexpr std::string_view VIEWER_WINDOW_NAME = "Map viewer";
	constexpr std::string_view CONFIG_WINDOW_NAME = "Map config";
	constexpr std::string_view VIEWER_CONFIG_WINDOW_NAME = "Map viewer config";
	constexpr room_gen_properties DEFAULT_GEN_PROPERTIES = {{100.f, 3.f, 2.f, 1.f, 4u, 5u, 20u},
	                                                        {20.f, 2.f, 1.f, 0.1f, 1u, 1u, 1u},
	                                                        10.5f,
	                                                        2.f,
	                                                        2.2f,
	                                                        0.5f};
} // namespace

map_tester::map_tester() noexcept
  : m_mt(std::random_device()())
  , m_gen_properties()
  , m_map_size{200, 100}
  , m_map(nullptr)
  , m_image()
  , m_texture()
  , m_from_pos()
  , wall_color(sf::Color::Blue)
  , empty_space_color(sf::Color::Black)
  , hole_color(sf::Color(122,122,122))
  , walkable_color(sf::Color(107,77,61))
  , none_color(sf::Color::Red)
{
	m_gen_properties.push_back(DEFAULT_GEN_PROPERTIES);
	updateMap();
}

void map_tester::configureDockspace(ImGuiID dockspace_id) const
{
	ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
	ImGui::DockBuilderAddNode(dockspace_id, ImGui::GetMainViewport()->Size); // Add empty node

	ImGuiID dock_main_id = dockspace_id;
	ImGuiID dock_id_left =
	  ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
	ImGuiID dock_id_down =
	  ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, nullptr, &dock_main_id);

	ImGui::DockBuilderDockWindow(VIEWER_WINDOW_NAME.data(), dock_main_id);
	ImGui::DockBuilderDockWindow(CONFIG_WINDOW_NAME.data(), dock_id_left);
	ImGui::DockBuilderDockWindow(VIEWER_CONFIG_WINDOW_NAME.data(), dock_id_down);
	ImGui::DockBuilderFinish(dockspace_id);
}

void map_tester::showConfigWindow()
{
	ImGui::Begin(CONFIG_WINDOW_NAME.data());

	int tmp = static_cast<int>(m_map_size.width);
	ImGui::InputInt("Map width", &tmp);
	m_map_size.width = static_cast<unsigned int>(tmp);
	tmp = static_cast<int>(m_map_size.height);
	ImGui::InputInt("Map height", &tmp);
	m_map_size.height = static_cast<unsigned int>(tmp);

	int properties_number = 0;
	for(room_gen_properties& properties: m_gen_properties)
	{
		const std::string properties_name =
		  std::string("Generation properties ") + std::to_string(++properties_number);
		ImGui::PushID(&properties_number);
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
		m_gen_properties.push_back(DEFAULT_GEN_PROPERTIES);
	}
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Button,
	                      static_cast<ImVec4>(ImColor::HSV(3.0f / 7.0f, 0.6f, 0.6f)));
	ImGui::Separator();
	if(ImGui::Button("Generate", ImVec2(ImGui::GetWindowContentRegionWidth(), 30)))
	{
		updateMap();
	}
	ImGui::PopStyleColor();

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
		ImGui::Text("From: (%d, %d)\nTo: (%d, %d)", m_from_pos.x, m_from_pos.y, pos.x, pos.y);
		ImGui::EndTooltip();

		if(ImGui::IsMouseClicked(0))
		{
			m_from_pos = pos;
		}
		if(ImGui::IsMouseClicked(1))
		{
			std::vector<dungeep::point_i> path = m_map->path_to_pt(m_from_pos, pos);
			updateMapView(); // clear last path
			for(const dungeep::point_i& pt: path)
			{
				m_image.setPixel(
				  static_cast<unsigned int>(pt.x), static_cast<unsigned int>(pt.y), none_color);
			}
			m_texture.loadFromImage(m_image);
		}
	}
	ImGui::End();
}

void map_tester::showViewerConfigWindow()
{
	ImGui::Begin(VIEWER_CONFIG_WINDOW_NAME.data());
	bool changed = false;
	changed |= showColorConfig("Wall", wall_color);
	changed |= showColorConfig("Empty space", empty_space_color);
	changed |= showColorConfig("Hole", hole_color);
	changed |= showColorConfig("Walkable", walkable_color);
	changed |= showColorConfig("None", none_color);
	changed |= ImGui::Button("Clear path");
	if(changed)
	{
		updateMapView();
	}
	ImGui::End();
}

void map_tester::updateMap()
{
	m_map = std::make_unique<map>(m_map_size, m_gen_properties, m_mt);
	updateMapView();
}

void map_tester::updateMapView()
{
	m_image.create(m_map_size.width, m_map_size.height);
	for(unsigned int i = 0; i < m_map_size.width; ++i)
	{
		for(unsigned int j = 0; j < m_map_size.height; ++j)
		{
			switch((*m_map)[i][j])
			{
				case tiles::wall:
					m_image.setPixel(i, j, wall_color);
					break;
				case tiles::empty_space:
					m_image.setPixel(i, j, empty_space_color);
					break;
				case tiles::hole:
					m_image.setPixel(i, j, hole_color);
					break;
				case tiles::walkable:
					m_image.setPixel(i, j, walkable_color);
					break;
				case tiles::none:
					m_image.setPixel(i, j, none_color);
					break;
			}
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
