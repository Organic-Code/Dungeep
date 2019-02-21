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
#include <cctype>
#include <imgui_internal.h>

#include <utils/logger.hpp>
#include <display/terminal.hpp>
#include <utils/resource_manager.hpp>
#include <utils/resource_keys.hpp>
#include <display/terminal_commands.hpp>
#include <environment/world.hpp>
#include <utils/misc.hpp>

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
			static constexpr std::array auto_complete_prime{1.f, 1.f, 1.f, 1.f};
			static constexpr std::array auto_complete_non_prime{0.8f, 0.8f, 0.8f, 1.f};
		};
	}
}

namespace {
	constexpr bool is_space(char c) {
		return c == ' ';
	}

	inline ImVec4 to_imgui_color(const std::array<float, 4>& color) {
		return ImVec4(color[0], color[1], color[2], color[3]);
	}

	class forwarding_sink final : public spdlog::sinks::base_sink<std::mutex> {
	protected:
		void sink_it_(const spdlog::details::log_msg &msg) override {
			fmt::memory_buffer buff;
			sink::formatter_->format(msg, buff);
			logger::sink->force_add_message(logger::message{msg.level, msg.color_range_start, msg.color_range_end, fmt::to_string(buff), true});
		}

		void flush_() override {}
	};
}

terminal::terminal()
	: autoscroll_text{resources::manager.get_text(keys::text::autoscroll)}
	, clear_text{resources::manager.get_text(keys::text::clear)}
	, log_level_text{resources::manager.get_text(keys::text::log_level)}
	, local_logger{cst::window_name, std::make_shared<forwarding_sink>()}
{
	std::fill(command_buffer.begin(), command_buffer.end(), '\0');

	local_logger.set_pattern("%T.%e - [%^command line%$]: %v");
	local_logger.info("welcome to command line.");
	local_logger.set_level(spdlog::level::trace);

	const std::string& trace = resources::manager.get_text(keys::text::trace);
	const std::string& debug = resources::manager.get_text(keys::text::debug);
	const std::string& info = resources::manager.get_text(keys::text::info);
	const std::string& warning = resources::manager.get_text(keys::text::warning);
	const std::string& error = resources::manager.get_text(keys::text::error);
	const std::string& critical = resources::manager.get_text(keys::text::critical);
	const std::string& none = resources::manager.get_text(keys::text::none);

	level_list_text.resize(trace.size() + 1 + debug.size() + 1 + info.size() + 1 + warning.size() + 1
	                       + error.size() + 1 + critical.size() + 1 + none.size() + 2, '\0');

	const std::string* levels[] = {&trace, &debug, &info, &warning, &error, &critical, &none };

	unsigned int current_shift = 0;
	longest_log_level = &trace;
	for (const std::string*& lvl : levels) {
		std::copy(lvl->begin(), lvl->end(), level_list_text.begin() + current_shift);
		current_shift += static_cast<unsigned int>(lvl->size()) + 1u;
		if (lvl->size() > longest_log_level->size()) {
			longest_log_level = lvl;
		}
	}

}

bool terminal::show() noexcept {
	should_show_next_frame = true;

	ImGui::SetNextWindowSize(ImVec2(cst::base_width, cst::base_height), ImGuiCond_Once);

	ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::foreground));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, to_imgui_color(cst::colors::background));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImGui::GetStyleColorVec4(ImGuiCol_TitleBg));

	if (!ImGui::Begin(cst::window_name)) {
		ImGui::End();
		ImGui::PopStyleColor(2);
		return true;
	}

	compute_text_size();
	display_settings_bar();
	display_messages();
	display_command_line();

	ImGui::End();
	ImGui::PopStyleColor(3);

	return should_show_next_frame;
}

void terminal::compute_text_size() noexcept {
	if (!selector_size_global) {
		selector_size_global = ImGui::CalcTextSize(longest_log_level->data());
		selector_label_size = ImGui::CalcTextSize(log_level_text.data());
		selector_label_size.x += 10;
		selector_size_global->x += selector_label_size.x + 30;
		selector_size_global->y += selector_label_size.y + 5;
		local_logger.info("x: {}, y: {}.", selector_size_global->x, selector_size_global->y);
	}
}

void terminal::display_settings_bar() noexcept {
	if (ImGui::Button(clear_text.data())) {
		logger::sink->clear();
	}
	ImGui::SameLine();
	ImGui::Checkbox(autoscroll_text.data(), &autoscroll);

	ImGui::SameLine();
	ImGui::BeginChild("terminal:settings:empty space"
			, ImVec2(ImGui::GetContentRegionAvailWidth() - selector_size_global.value().x, selector_size_global.value().y));
	ImGui::EndChild();

	ImGui::SameLine();
	if (ImGui::BeginChild("terminal:settings:log level selector", selector_size_global.value())) {
		ImGui::TextUnformatted(log_level_text.data(), log_level_text.data() + log_level_text.size());

		ImGui::SameLine();
		ImGui::PushItemWidth(-5);
		ImGui::Combo("##terminal:log_level_selector:combo", &level, level_list_text.data());
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
}

void terminal::display_messages() noexcept {
	ImVec2 avail_space = ImGui::GetContentRegionAvail();
	if (avail_space.y > selector_size_global->y) {
		if (ImGui::BeginChild("terminal:logs_window", ImVec2(avail_space.x, avail_space.y - selector_size_global->y), false,
		                      ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar)) {
			for (const logger::message& msg : *logger::sink) {
				if (msg.severity < level && !msg.important) {
					continue;
				}

				if (msg.color_beg < msg.color_end) {
					ImGui::TextUnformatted(msg.value.data(), msg.value.data() + msg.color_beg);
					ImGui::SameLine(0.f, 0.f);
					ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::log_level_colors[msg.severity]));
					ImGui::TextUnformatted(msg.value.data() + msg.color_beg, msg.value.data() + msg.color_end);
					ImGui::PopStyleColor();
					ImGui::SameLine(0.f, 0.f);
					ImGui::TextUnformatted(msg.value.data() + msg.color_end, msg.value.data() + msg.value.size());
				} else {
					ImGui::TextUnformatted(msg.value.data(), msg.value.data() + msg.value.size());
				}
			}
		}
		if (autoscroll) {
			if (last_size != logger::sink->size()) {
				ImGui::SetScrollHereY(1.f);
				last_size = logger::sink->size();
			}
		} else {
			last_size = 0u;
		}

		ImGui::EndChild();
	}
}

void terminal::display_command_line() noexcept {
	if (!command_entered && ImGui::GetActiveID() == input_text_id && input_text_id != 0 && current_autocomplete.empty() && buffer_usage == 0u) {
		current_autocomplete = commands::list_commands();
	}

	ImGui::Separator();
	show_input_text();
	handle_unfocus();
	show_autocomplete();
}

void terminal::show_input_text() noexcept {
	ImGui::PushItemWidth(-1.f);
	if (ImGui::InputText("##terminal:input_text", command_buffer.data(), command_buffer.size()
			, ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, terminal::command_line_callback, this)) {
		if (buffer_usage > 0u && command_buffer[buffer_usage - 1] == '\0') {
			--buffer_usage;
		} else if (buffer_usage + 1 < command_buffer.size() && command_buffer[buffer_usage + 1] == '\0' && command_buffer[buffer_usage] != '\0'){
			++buffer_usage;
		} else {
			buffer_usage = std::strlen(command_buffer.data());
		}

		auto beg = std::find_if_not(command_buffer.begin(), command_buffer.begin() + buffer_usage, is_space);
		auto ed = std::find_if(beg, command_buffer.begin() + buffer_usage, is_space);

		if (ed == command_buffer.begin() + buffer_usage) {
			current_autocomplete = commands::find_by_prefix(beg, ed);
			command_entered = false;
		} else {
			// TODO: advanced auto completion
			command_entered = true;
			current_autocomplete.clear();
		}
	}
	ImGui::PopItemWidth();

	if (input_text_id == 0u) {
		input_text_id = ImGui::GetItemID();
	}
}

void terminal::handle_unfocus() noexcept {
	if (previously_active_id == input_text_id) {
		if (ImGui::IsKeyPressedMap(ImGuiKey_Enter)) {
			call_command();
			command_buffer[0] = '\0';
			buffer_usage = 0u;
			command_entered = false;
		} else if (ImGui::IsKeyPressedMap(ImGuiKey_Escape)) {
			current_autocomplete.clear();
			if (buffer_usage > 0u) {
				command_buffer[0] = '\0';
				buffer_usage = 0u;
			} else {
				should_show_next_frame = false; // should hide on next frames
			}
			command_entered = false;
		}
	}
	previously_active_id = ImGui::GetActiveID();
}

void terminal::show_autocomplete() noexcept {
	constexpr ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar
	                                           | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize
	                                           | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (input_text_id == ImGui::GetActiveID() && !current_autocomplete.empty()) {
		ImGui::SetNextWindowBgAlpha(0.9f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::SetNextWindowFocus();

		ImVec2 auto_complete_pos = ImGui::GetItemRectMin();
		auto_complete_pos.y -= (selector_size_global->y + 3);
		ImGui::SetNextWindowPos(auto_complete_pos);
		if (ImGui::Begin("##terminal:auto_complete", nullptr, overlay_flags)) {

			const std::string_view& first = current_autocomplete[0].get().name;
			ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::auto_complete_prime));
			ImGui::TextUnformatted(first.data(), first.data() + first.size());

			ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::auto_complete_non_prime));
			for (auto it = std::next(current_autocomplete.cbegin()), end = current_autocomplete.cend() ; it != end ; ++it) {
				ImGui::SameLine();
				ImGui::VerticalSeparator();
				ImGui::SameLine();
				ImGui::TextDisabled("%s", it->get().name.data());
			}
			ImGui::PopStyleColor(2);
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
}

void terminal::call_command() noexcept {
	auto command_beg = std::find_if_not(command_buffer.begin(), command_buffer.begin() + buffer_usage, is_space);
	auto command_ed = std::find_if(command_beg, command_buffer.begin() + buffer_usage, is_space);
	std::vector<std::reference_wrapper<const commands::list_element_t>> matching_command_list{};

	matching_command_list = commands::find_by_prefix(command_beg, command_ed);

	if (matching_command_list.empty()) {
		local_logger.error("\"{}\": command not found.", std::string_view(&*command_beg, command_ed - command_beg));
		return;
	}
	auto argument_beg = std::find_if_not(command_ed, command_buffer.begin() + buffer_usage, is_space);
	auto argument_end = std::find_if_not(std::reverse_iterator(command_buffer.begin() + buffer_usage)
			                           , std::reverse_iterator(argument_beg)
			                           , is_space).base();

	// TODO: REMOVE
	//vvvvv
	world w;
	matching_command_list[0].get().call(commands::argument{w, *this, std::string_view{command_buffer.begin(), buffer_usage}
	, std::string_view{argument_beg, static_cast<unsigned long>(argument_end - argument_beg)}});
}

int terminal::command_line_callback(ImGuiInputTextCallbackData* data) noexcept {
	auto t = reinterpret_cast<terminal *>(data->UserData);
	if (data->EventKey == ImGuiKey_Tab) {

		if (t->current_autocomplete.empty()) {
			return 0;
		}

		std::string_view complete = t->current_autocomplete[0].get().name;

		auto command_beg = std::find_if_not(t->command_buffer.begin(), t->command_buffer.begin() + t->buffer_usage, is_space);
		unsigned long leading_space = command_beg - t->command_buffer.begin();

		misc::copy(complete.data() + t->buffer_usage - leading_space, complete.data() + complete.size()
				, data->Buf + t->buffer_usage, data->Buf + data->BufSize - 1);

		auto buf_size = std::min(complete.size() + leading_space, static_cast<unsigned long>(data->BufSize - 1));
		data->Buf[buf_size] = '\0';
		data->BufTextLen = static_cast<int>(buf_size);

		data->BufDirty = true;
		data->SelectionStart = data->SelectionEnd;
		data->CursorPos = data->BufTextLen;
		t->current_autocomplete.clear();
	} else {
		// TODO
		if (data->EventKey == ImGuiKey_UpArrow) {

		} else if (data->EventKey == ImGuiKey_DownArrow) {

		} else {
			t->local_logger.warn("Unknown key event catched by autocompletion system: {}", data->EventKey);
		}
	}




	return 0;
}
