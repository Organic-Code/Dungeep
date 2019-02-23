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
#include <charconv>

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
			static constexpr std::array auto_complete_selected{1.f, 1.f, 1.f, 1.f};
			static constexpr std::array auto_complete_non_selected{0.5f, 0.45f, 0.45f, 1.f};
			static constexpr std::array auto_complete_separator{0.6f, 0.6f, 0.6f, 1.f};
			static constexpr std::array msg_bg{0.1f, 0.1f, 0.1f, 1.f};
		};
	}
}

namespace {
	inline ImVec4 to_imgui_color(const std::array<float, 4>& color) {
		return ImVec4(color[0], color[1], color[2], color[3]);
	}

	class forwarding_sink final : public spdlog::sinks::base_sink<std::mutex> {
	public:
		forwarding_sink() {
			non_trace_pattern();
		}
	protected:
		void sink_it_(const spdlog::details::log_msg &msg) override {
			if (msg.level == spdlog::level::trace) {
				if (!is_trace_pattern) {
					trace_pattern();
				}
			} else if (is_trace_pattern) {
				non_trace_pattern();
			}
			fmt::memory_buffer buff;
			sink::formatter_->format(msg, buff);
			logger::sink->force_add_message(logger::message{msg.level, msg.color_range_start, msg.color_range_end, fmt::to_string(buff), true});
		}

		void flush_() override {}

	private:

		void non_trace_pattern() {
			set_pattern_("%T.%e - [%^command line%$]: %v");
			is_trace_pattern = false;
		}
		void trace_pattern() {
			set_pattern_("%T.%e - %^%v%$");
			is_trace_pattern = true;
		}

		bool is_trace_pattern{false};
	};
}

terminal::terminal()
	: local_logger{cst::window_name, std::make_shared<forwarding_sink>()}
	, autoscroll_text{resources::manager.get_text(keys::text::autoscroll)}
	, clear_text{resources::manager.get_text(keys::text::clear)}
	, log_level_text{resources::manager.get_text(keys::text::log_level)}
{
	std::fill(command_buffer.begin(), command_buffer.end(), '\0');

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
	should_show_next_frame = !close_request;
	close_request = false;

	ImGui::SetNextWindowSize(ImVec2(cst::base_width, cst::base_height), ImGuiCond_Once);

	ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::foreground));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, to_imgui_color(cst::colors::background));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImGui::GetStyleColorVec4(ImGuiCol_TitleBg));

	if (!ImGui::Begin(cst::window_name, nullptr, ImGuiWindowFlags_NoScrollbar)) {
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
		ImGui::PushStyleColor(ImGuiCol_ChildBg, to_imgui_color(cst::colors::msg_bg));
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
		ImGui::PopStyleColor();
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
	if (should_take_focus) {
		ImGui::SetKeyboardFocusHere();
		should_take_focus = false;
	}
	if (ImGui::InputText("##terminal:input_text", command_buffer.data(), command_buffer.size(),
	                     ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
	                     terminal::command_line_callback_st, this) && !ignore_next_textinput) {
		current_history_selection = {};
		if (buffer_usage > 0u && command_buffer[buffer_usage - 1] == '\0') {
			--buffer_usage;
		} else if (buffer_usage + 1 < command_buffer.size() && command_buffer[buffer_usage + 1] == '\0' && command_buffer[buffer_usage] != '\0'){
			++buffer_usage;
		} else {
			buffer_usage = std::strlen(command_buffer.data());
		}

		auto beg = std::find_if_not(command_buffer.begin(), command_buffer.begin() + buffer_usage, misc::is_space);
		auto ed = std::find_if(beg, command_buffer.begin() + buffer_usage, misc::is_space);

		if (ed == command_buffer.begin() + buffer_usage) {
			current_autocomplete = commands::find_by_prefix(beg, ed);
			command_entered = false;
		} else {
			// TODO: advanced auto completion
			command_entered = true;
			current_autocomplete.clear();
		}
	}
	ignore_next_textinput = false;
	ImGui::PopItemWidth();

	if (input_text_id == 0u) {
		input_text_id = ImGui::GetItemID();
	}
}

void terminal::handle_unfocus() noexcept {
	auto clear_frame = [this]() {
		command_buffer[0] = '\0';
		buffer_usage = 0u;
		command_line_backup_prefix.remove_prefix(command_line_backup_prefix.size());
		command_line_backup.clear();
		current_history_selection = {};
		command_entered = false;
		current_autocomplete.clear();
	};

	if (previously_active_id == input_text_id && ImGui::GetActiveID() != input_text_id) {
		if (ImGui::IsKeyPressedMap(ImGuiKey_Enter)) {
			call_command();
			should_take_focus = true;
			clear_frame();

		} else if (ImGui::IsKeyPressedMap(ImGuiKey_Escape)) {
			if (buffer_usage == 0u) {
				should_show_next_frame = false; // should hide on next frames
			}
			clear_frame();
		}
	}
	previously_active_id = ImGui::GetActiveID();
}

void terminal::show_autocomplete() noexcept {
	constexpr ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar
	                                           | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize
	                                           | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if ((input_text_id == ImGui::GetActiveID() || should_take_focus) && !current_autocomplete.empty()) {

		ImGui::SetNextWindowBgAlpha(0.9f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::SetNextWindowFocus();

		ImVec2 auto_complete_pos = ImGui::GetItemRectMin();
//		auto_complete_pos.y -= (selector_size_global->y + 3);
		auto_complete_pos.y = ImGui::GetItemRectMax().y;

		ImVec2 auto_complete_max_size = ImGui::GetItemRectSize();
		auto_complete_max_size.y = -1.f;
		ImGui::SetNextWindowPos(auto_complete_pos);
		ImGui::SetNextWindowSizeConstraints({0.f, 0.f}, auto_complete_max_size);
		if (ImGui::Begin("##terminal:auto_complete", nullptr, overlay_flags)) {

			auto print_separator = [this]() {
				ImGui::SameLine(0.f, 0.f);
				ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::auto_complete_separator));
				ImGui::TextUnformatted(autocomlete_separator.data(),
				                       autocomlete_separator.data() + autocomlete_separator.size());
				ImGui::PopStyleColor();
				ImGui::SameLine(0.f, 0.f);
			};

			int max_displayable_sv = 0;
			float separator_length = ImGui::CalcTextSize(autocomlete_separator.data(),
			                                             autocomlete_separator.data() + autocomlete_separator.size()).x;
			float total_text_length = ImGui::CalcTextSize("...").x;
			for (const commands::list_element_t& sv : current_autocomplete) {
				float t_len = ImGui::CalcTextSize(sv.name.data(), sv.name.data() + sv.name.size()).x + separator_length;
				if (t_len + total_text_length < auto_complete_max_size.x) {
					total_text_length += t_len;
					++max_displayable_sv;
				} else {
					break;
				}
			}

			std::string_view last;
			if (max_displayable_sv != 0) {
				const std::string_view& first = current_autocomplete[0].get().name;
				ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::auto_complete_selected));
				ImGui::TextUnformatted(first.data(), first.data() + first.size());
				ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::auto_complete_non_selected));
				for (int i = 1 ; i < max_displayable_sv ; ++i) {
					const std::string_view vs = current_autocomplete[i].get().name;
					print_separator();
					ImGui::TextUnformatted(vs.data(), vs.data() + vs.size());
				}
				ImGui::PopStyleColor(2);
				if (max_displayable_sv < static_cast<long>(current_autocomplete.size())) {
					last = current_autocomplete[max_displayable_sv].get().name;
				}
			}

			if (max_displayable_sv < static_cast<long>(current_autocomplete.size())) {

				if (max_displayable_sv == 0) {
					last = current_autocomplete.front().get().name;
					ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::auto_complete_selected));
					total_text_length -= separator_length;
				} else {
					ImGui::PushStyleColor(ImGuiCol_Text, to_imgui_color(cst::colors::auto_complete_non_selected));
					print_separator();
				}

				std::vector<char> buf;
				buf.resize(last.size() + 4);
				std::copy(last.begin(), last.end(), buf.begin());
				std::fill(buf.begin() + last.size(), buf.end(), '.');
				auto size = static_cast<unsigned>(last.size() + 3);
				while (size >= 4 && total_text_length + ImGui::CalcTextSize(buf.data(), buf.data() + size).x >= auto_complete_max_size.x) {
					buf[size - 4] = '.';
					--size;
				}
				while (size != 0 && total_text_length + ImGui::CalcTextSize(buf.data(), buf.data() + size).x >= auto_complete_max_size.x) {
					--size;
				}
				ImGui::TextUnformatted(buf.data(), buf.data() + size);
				ImGui::PopStyleColor();
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
}

void terminal::call_command() noexcept {
	std::optional<std::vector<std::string>> splitted = misc::split_by_space({command_buffer.begin(), buffer_usage});

	if (!splitted) {
		local_logger.trace("{}", std::string_view{command_buffer.begin(), buffer_usage});
		local_logger.error(R"(Unmatched ")");
		return;
	}

	if (splitted->empty()) {
		local_logger.trace("{}", std::string_view{command_buffer.begin(), buffer_usage});
		return;
	}

	bool modified{};
	std::optional<std::string_view> solved = resolve_history_reference((*splitted)[0], modified);
	if (!solved) {
		local_logger.error(R"("{}": no such event.)", (*splitted)[0]);
		return;
	}

	auto concat = [](std::string& str, const std::string_view& sv) {

		if (sv.empty()) {
			str += R"("")";
		} else if (misc::is_space(sv[0]) || misc::is_space(sv[sv.size() - 1])) {
			str += '"';
			str += sv;
			str += '"';
		} else {
			str += sv;
		}
	};

	std::string global_command;
	concat(global_command, *solved);
	(*splitted)[0] = *solved;

	for (auto it = std::next(splitted->begin()) ; it != splitted->end() ; ++it) {
		bool inner_modified{};
		solved = resolve_history_reference(*it, inner_modified);
		if (!solved) {
			local_logger.error(R"("{}": no such event.)", *it);
			return;
		}
		*it = *solved;
		global_command.reserve(global_command.size() + solved->size() + 1);
		global_command += ' ';
		concat(global_command, *solved);
		modified |= inner_modified;
	}

	local_logger.trace("{}", std::string_view{command_buffer.begin(), buffer_usage});

	if (modified) {
		splitted = misc::split_by_space(global_command);

		if (!splitted) {
			local_logger.error(R"(Unmatched ")");
			return;
		}

		if (splitted->empty()) {
			local_logger.trace("> {}", global_command);
			return;
		}

		std::string global_command_reformed;
		global_command_reformed.reserve(global_command.size());
		for (const std::string& vs : *splitted) {
			if (vs.empty()) {
				global_command_reformed += R"("")";
			} else if (vs.front() == ' ' || vs.back() == ' ') {
				global_command_reformed += '"';
				global_command_reformed += vs;
				global_command_reformed += '"';
			} else {
				global_command_reformed += vs;
			}
			global_command_reformed += ' ';
		}

		local_logger.trace("> {}", global_command_reformed);
		command_history.emplace_back(std::move(global_command_reformed));
	} else {
		command_history.emplace_back(command_buffer.begin(), buffer_usage);
	}

	std::vector<std::reference_wrapper<const commands::list_element_t>> matching_command_list{};
	matching_command_list = commands::find_by_prefix((*splitted)[0]);

	if (matching_command_list.empty()) {
		local_logger.error(R"("{}": command not found.)", splitted->front());
		return;
	}


	// TODO: REMOVE
	//vvvvv
	world w;
	matching_command_list[0].get().call({w, *this, *splitted});
}

std::optional<std::string_view> terminal::resolve_history_reference(std::string_view str, bool& modified) const noexcept {
	modified = false;

	if (str.empty() || str[0] != '!') {
		return str;
	}

	if (str.size() < 2) {
		return {};
	}

	if (str[1] == '!') {
		if (command_history.empty() || str.size() != 2) {
			return {};
		} else {
			modified = true;
			return {command_history.back()};
		}
	}

	unsigned int backward_jump = 1;
	unsigned int char_idx = 1;
	if (str[1] == '-') {
		if (str.size() <= 2 || !misc::is_digit(str[2])) {
			return {};
		}

		unsigned int val{0};
		std::from_chars_result res = std::from_chars(str.data() + 2, str.data() + str.size(), val, 10);
		if (val == 0) {
			return {}; // val == 0  <=> (garbage input || user inputted 0)
		}

		backward_jump = val;
		char_idx = static_cast<unsigned int>(res.ptr - str.data());
	}

	if (command_history.size() < backward_jump) {
		return {};
	}

	if (char_idx == str.size()) {
		return command_history[command_history.size() - backward_jump];
	}

	if (char_idx > str.size() || str[char_idx] != ':') {
		return {};
	}


	++char_idx;
	if (str.size() <= char_idx || !misc::is_digit(str[char_idx])) {
		return {};
	}

	unsigned int val1{};
	std::from_chars_result res1 = std::from_chars(str.data() + char_idx, str.data() + str.size(), val1, 10);
	if (!misc::success(res1.ec)) {
		return {};
	}

	const std::string& cmd = command_history[command_history.size() - backward_jump]; // 1 <= backward_jump <= command_history.size()
	std::optional<std::vector<std::string>> args = misc::split_by_space(cmd);

	if (!args || args->size() <= val1) {
		return {};
	}

	modified = true;
	return (*args)[val1];

}

int terminal::command_line_callback_st(ImGuiInputTextCallbackData * data) noexcept {
	return reinterpret_cast<terminal *>(data->UserData)->command_line_callback(data);
}

int terminal::command_line_callback(ImGuiInputTextCallbackData* data) noexcept {

	auto paste_buffer = [data](auto begin, auto end, auto buffer_shift) {
		misc::copy(begin, end, data->Buf + buffer_shift, data->Buf + data->BufSize - 1);
		data->BufTextLen = std::min(static_cast<int>(std::distance(begin, end) + buffer_shift), data->BufSize - 1);
		data->Buf[data->BufTextLen] = '\0';
		data->BufDirty = true;
		data->SelectionStart = data->SelectionEnd;
		data->CursorPos = data->BufTextLen;
	};

	if (data->EventKey == ImGuiKey_Tab) {

		if (current_autocomplete.empty()) {
			return 0;
		}

		std::string_view complete = current_autocomplete[0].get().name;

		auto command_beg = std::find_if_not(command_buffer.begin(), command_buffer.begin() + buffer_usage, misc::is_space);
		unsigned long leading_space = command_beg - command_buffer.begin();
		paste_buffer(complete.data() + buffer_usage - leading_space, complete.data() + complete.size(), buffer_usage);
		buffer_usage = static_cast<unsigned>(data->BufTextLen);
		current_autocomplete.clear();

	} else if (data->EventKey == ImGuiKey_UpArrow) {
		if (command_history.empty()) {
			return 0;
		}
		ignore_next_textinput = true;

		if (!current_history_selection) {
			current_history_selection = command_history.end();
			command_line_backup = std::string(command_buffer.begin(), command_buffer.begin() + buffer_usage);
			command_line_backup_prefix = command_line_backup;
			command_line_backup_prefix.remove_prefix(std::min(command_line_backup_prefix.find_first_not_of(" "), command_line_backup_prefix.size()));
			current_autocomplete.clear();
		}

		auto it = misc::find_first_prefixed(
				command_line_backup_prefix
				, std::reverse_iterator(*current_history_selection)
				, command_history.rend());

		if (it != command_history.rend()) {
			current_history_selection = std::prev(it.base());
			paste_buffer((*current_history_selection)->begin() + command_line_backup_prefix.size()
					, (*current_history_selection)->end(), command_line_backup.size());
			buffer_usage = static_cast<unsigned>(data->BufTextLen);
		} else {
			if (current_history_selection == command_history.end()) {
				// no auto completion occured
				ignore_next_textinput = false;
				current_history_selection = {};
				command_line_backup_prefix = {};
				command_line_backup.clear();
			}
		}

	} else if (data->EventKey == ImGuiKey_DownArrow) {

		if (!current_history_selection) {
			return 0;
		}
		ignore_next_textinput = true;

		current_history_selection = misc::find_first_prefixed(
				command_line_backup_prefix
				, std::next(*current_history_selection)
				, command_history.end());

		if (current_history_selection != command_history.end()) {
			paste_buffer((*current_history_selection)->begin() + command_line_backup_prefix.size()
					, (*current_history_selection)->end(), command_line_backup.size());
			buffer_usage = static_cast<unsigned>(data->BufTextLen);

		} else {
			data->BufTextLen = static_cast<int>(command_line_backup.size());
			data->Buf[data->BufTextLen] = '\0';
			data->BufDirty = true;
			data->SelectionStart = data->SelectionEnd;
			data->CursorPos = data->BufTextLen;
			buffer_usage = static_cast<unsigned>(data->BufTextLen);

			current_history_selection = {};
			command_line_backup_prefix = {};
			command_line_backup.clear();
		}

	} else {
		local_logger.warn("Unexpected event thrown to autocompletion system: {}", data->EventFlag);
	}

	return 0;
}
