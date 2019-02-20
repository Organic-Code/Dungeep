///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2019, Lucas Lazare                                                                                                     ///
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

#include <imgui.h>
#include <array>
#include <utils/logger.hpp>
#include <display/terminal.hpp>
#include <utils/resource_manager.hpp>
#include <utils/resource_keys.hpp>

#include "display/terminal.hpp"

namespace cst {
	namespace {
		constexpr int base_width = 900;
		constexpr int base_height = 200;

		constexpr const char* window_name = "terminal";

		struct colors {
			static_assert(spdlog::level::trace    == 0);
			static_assert(spdlog::level::debug    == 1);
			static_assert(spdlog::level::info     == 2);
			static_assert(spdlog::level::warn     == 3);
			static_assert(spdlog::level::err      == 4);
			static_assert(spdlog::level::critical == 5);

			static constexpr std::array<std::array<float,4>,6> log_level_colors
			{
				std::array{0.549f, 0.561f, 0.569f, 1.f},
				std::array{0.153f, 0.596f, 0.498f, 1.f},
				std::array{0.459f, 0.686f, 0.129f, 1.f},
				std::array{0.839f, 0.749f, 0.333f, 1.f},
				std::array{1.000f, 0.420f, 0.408f, 1.f},
				std::array{1.000f, 0.420f, 0.408f, 1.f},
			};

			static constexpr std::array background{0.170f, 0.170f, 0.170f, 1.f};
			static constexpr std::array foreground{0.627f, 0.678f, 0.698f, 1.f};
		};
	}
}

namespace {
	inline ImVec4 to_imgui_color(const std::array<float, 4>& color) {
		return ImVec4(color[0], color[1], color[2], color[3]);
	}

	class forwarding_sink final : public spdlog::sinks::base_sink<std::mutex> {
	protected:
		void sink_it_(const spdlog::details::log_msg &msg) override {
			fmt::memory_buffer buff;
			sink::formatter_->format(msg, buff);
			logger::sink->force_add_message(logger::message{msg.level, msg.color_range_start, msg.color_range_end, fmt::to_string(buff)});
		}

		void flush_() override {}
	};
}

terminal::terminal()
	: local_logger{cst::window_name, std::make_shared<forwarding_sink>()}
	, autoscroll_text{resources::manager.get_text(keys::text::autoscroll)}
	, clear_text{resources::manager.get_text(keys::text::clear)}
	, log_level_text{resources::manager.get_text(keys::text::log_level)}
	{

	local_logger.set_pattern("%T.%e - [%^command line%$]: %v");
	local_logger.info("welcome to command line.");
	const std::string& trace = resources::manager.get_text(keys::text::trace);
	const std::string& debug = resources::manager.get_text(keys::text::debug);
	const std::string& info = resources::manager.get_text(keys::text::info);
	const std::string& warning = resources::manager.get_text(keys::text::warning);
	const std::string& error = resources::manager.get_text(keys::text::error);
	const std::string& critical = resources::manager.get_text(keys::text::critical);
	const std::string& none = resources::manager.get_text(keys::text::none);

	level_list_text.resize(trace.size() + 1 + debug.size() + 1 + info.size() + 1 + warning.size() + 1
	                       + error.size() + 1 + critical.size() + 1 + none.size() + 2, '\0');

	std::copy(trace.begin(), trace.end(), level_list_text.data());
	std::copy(debug.begin(), debug.end(), level_list_text.data() + trace.size() + 1);
	std::copy(info.begin(), info.end(), level_list_text.data() + trace.size() + 1 + debug.size() + 1);
	std::copy(warning.begin(), warning.end(), level_list_text.data() + trace.size() + 1 + debug.size() + 1 + info.size() + 1);
	std::copy(error.begin(), error.end(), level_list_text.data() + trace.size() + 1 + debug.size() + 1 + info.size() + 1 + warning.size() + 1);
	std::copy(critical.begin(), critical.end(), level_list_text.data() + trace.size() + 1 + debug.size() + 1 + info.size() + 1 + warning.size() + 1 + error.size() + 1);
	std::copy(none.begin(), none.end(), level_list_text.data() + trace.size() + 1 + debug.size() + 1 + info.size() + 1 + warning.size() + 1 + error.size() + 1 + critical.size() + 1);
}

void terminal::show() {

	ImGui::PushStyleColor(ImGuiCol_WindowBg, to_imgui_color(cst::colors::background));
	ImGui::SetNextWindowSize(ImVec2(cst::base_width, cst::base_height), ImGuiCond_Once);

	if (!ImGui::Begin(cst::window_name)) {
		ImGui::End();
		ImGui::PopStyleColor();
		return;
	}

	ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::foreground));

	ImGui::Checkbox(autoscroll_text.data(), &autoscroll);
	ImGui::SameLine();
	ImGui::Combo(log_level_text.data(), &level, level_list_text.data());
	ImGui::SameLine();
	if (ImGui::Button(clear_text.data())) {
		logger::sink->clear();
	}

	if (ImGui::BeginChild("terminal:logs_window", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar)) {
		for (const logger::message& msg : *logger::sink) {
			if (msg.severity < level) {
				continue;
			}

			if (msg.color_beg < msg.color_end) {
				ImGui::TextUnformatted(msg.value.data(), msg.value.data() + msg.color_beg);
				ImGui::SameLine(0.f, 0.f);
				ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::log_level_colors[msg.severity]));
				ImGui::TextUnformatted(msg.value.data() + msg.color_beg, msg.value.data() + msg.color_end);
				ImGui::PopStyleColor();
				ImGui::SameLine(0.f, 0.f);
				ImGui::TextUnformatted(msg.value.data() + msg.color_end);
			} else {
				ImGui::TextUnformatted(msg.value.data());
			}
		}
	}
	if (autoscroll && last_size != logger::sink->size()) {
		ImGui::SetScrollHereY(1.f);
		last_size = logger::sink->size();
	}

	ImGui::EndChild();
	ImGui::End();
	ImGui::PopStyleColor(2);
}
