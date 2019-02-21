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
#include <utils/logger.hpp>
#include <display/terminal.hpp>

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
	logger::log.trace("Hello, World!");
	logger::log.debug("Hello, World!");
	logger::log.info("Hello, World!");
	logger::log.warn("Hello, World!");
	logger::log.error("Hello, World!");
	logger::log.critical("Hello, World!");

	auto seed = std::random_device()();
	std::mt19937_64 mt{seed};
	std::array<proba_tester, 8> t;
	std::array<dungeep::normal_distribution<float>, t.size() / 2> norm_dists{
			dungeep::normal_distribution{500.f, 40.f},
			dungeep::normal_distribution{470.f, 40.f},
			dungeep::normal_distribution{500.f, 100.f},
			dungeep::normal_distribution{500.f, 4.f}
	};
	std::array<dungeep::uniform_int_distribution<int>, (t.size() + 1)/ 2> uni_dists{
			dungeep::uniform_int_distribution{0, 1000},
			dungeep::uniform_int_distribution{460, 510},
			dungeep::uniform_int_distribution{490, 600},
			dungeep::uniform_int_distribution{500, 504}
	};

	auto re_maker = [&mt](auto& distrib) {
		return [&mt, &distrib] {
			return static_cast<int>(std::round(distrib(mt)));
		};
	};

	for (auto i = 0u ; i < t.size() / 2; ++i) {
		t[i].test_distribution(re_maker(norm_dists[i]), 0, 1000, 1000u);
		t[i + t.size() / 2].test_distribution(re_maker(uni_dists[i]), 0, 1000, 1000u);
	}

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

	terminal terminal_log;

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

  		for (proba_tester& proba_tester : t) {
  			proba_tester.update();
  		}

		showMainDockSpace(tester);
		tester.showViewerWindow();
		tester.showConfigWindow();
		tester.showViewerConfigWindow();
		tester.showDebugInfoWindow();

		terminal_log.show();

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
