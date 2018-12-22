#include <iostream>
#include "map.hpp"
#include "quadtree.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <iostream>
#include <map_tester.hpp>

namespace
{
	constexpr const char* WINDOW_NAME = "Map tester";
	constexpr unsigned int WINDOW_INITIAL_WIDTH = 600;
	constexpr unsigned int WINDOW_INITIAL_HEIGHT = 400;
	constexpr unsigned int FRAME_RATE_LIMIT = 60;

	void showMainDockSpace(const map_tester& map_tester);
} // namespace

int main()
{
	map_tester tester;

	sf::Clock deltaClock;
	sf::Clock musicOffsetClock;

	sf::RenderWindow window(sf::VideoMode(WINDOW_INITIAL_WIDTH, WINDOW_INITIAL_HEIGHT),
	                        WINDOW_NAME //, sf::Style::Titlebar | sf::Style::Close
	);
	window.setFramerateLimit(FRAME_RATE_LIMIT);
	ImGui::SFML::Init(window);

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
	io.ConfigDockingWithShift = true; // hold shift to use docking
	io.IniFilename = nullptr; // disable .ini saving

	while(window.isOpen())
	{
		sf::Event event{};
		while(window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if(event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		showMainDockSpace(tester);
		tester.showViewerWindow();
		tester.showConfigWindow();
		tester.showViewerConfigWindow();
		tester.showDebugInfoWindow();

		window.clear();
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
	return EXIT_SUCCESS;
}

namespace
{

	void showMainDockSpace(const map_tester& map_tester)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Main dockspace",
		             nullptr,
		             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
		               | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
		               | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar
		               | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus
		               | ImGuiWindowFlags_NoNavFocus);
		ImGui::PopStyleVar(2);

		const ImGuiID dockspace_id = ImGui::GetID("main dockspace");
		if(ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
		{
			map_tester.configureDockspace(dockspace_id);
		}
		ImGui::DockSpace(dockspace_id);
		//nothing
		ImGui::End();
	}
} // namespace
