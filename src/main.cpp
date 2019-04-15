///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018-2019, Lucas Lazare                                                                                                ///
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


#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <map>
#include <utils/random.hpp>
#include <display/proba_tester.hpp>

#include "display/map_tester.hpp"

#include <spdlog/spdlog.h>
#include <imterm/terminal.hpp>
#include <display/terminal_commands.hpp>
#include <environment/world.hpp>
#include <utils/resource_manager.hpp>

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
	program_state prgm;
	ImTerm::terminal<terminal_commands> terminal_log(prgm, "terminal", WINDOW_INITIAL_WIDTH);
	terminal_log.theme() = ImTerm::themes::cherry;
	terminal_log.log_level(ImTerm::message::severity::info);
	terminal_log.set_flags(ImGuiWindowFlags_NoTitleBar);
	terminal_log.disallow_x_resize();
	terminal_log.filter_hint() = "regex filter...";
	bool showing_term = true;

	spdlog::default_logger()->sinks().push_back(terminal_log.get_terminal_helper());
	spdlog::default_logger()->set_level(spdlog::level::trace);

	spdlog::info("~ Welcome to Dungeep ~");

	resources::manager = std::make_unique<resources>();

	map_tester tester;

	sf::Clock deltaClock;
	sf::Clock musicOffsetClock;

	sf::RenderWindow window({WINDOW_INITIAL_WIDTH, WINDOW_INITIAL_HEIGHT}, WINDOW_NAME);
	bool resized_once = false;
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
			} else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::F11) {
					showing_term = !showing_term;
				}
			} else if (event.type == sf::Event::Resized) {
				terminal_log.set_width(window.getSize().x);
				if (resized_once) {
					terminal_log.set_height(std::min(window.getSize().y, static_cast<unsigned>(terminal_log.get_size().y)));
				}
				resized_once = true;
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		showMainDockSpace(tester);
		tester.showViewerWindow();
		tester.showConfigWindow();
		tester.showViewerConfigWindow();
		tester.showDebugInfoWindow();

		if (showing_term) {
			ImGui::SetNextWindowPos({0.f, 0.f}, ImGuiCond_Always);
			showing_term = terminal_log.show();
			if (!prgm.running) {
				window.close();
			}
		}

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
		if(ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
			map_tester.configureDockspace(dockspace_id);
		}
		ImGui::DockSpace(dockspace_id);
		ImGui::End();
	}
} // namespace
