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

#ifndef DUNGEEP_TERMINAL_HPP
#define DUNGEEP_TERMINAL_HPP

#include <vector>
#include <string>
#include <spdlog/spdlog.h>

#include <imgui.h>

namespace commands {
	struct list_element_t;
}

class terminal {
public:
	using buffer_type = std::array<char, 1024>;

	explicit terminal(const char * window_name_ = "terminal", int base_widh_ = 900, int base_height_ = 200);

	bool show() noexcept;

	void hide() noexcept {
		previously_active_id = 0;
	}

	const std::vector<std::string>& get_history() const noexcept {
		return command_history;
	}

	spdlog::logger& command_log() noexcept {
		return local_logger;
	}

	void set_should_close() noexcept {
		close_request = true;
	}

	void reset_colors() noexcept {
		for (std::optional<ImVec4>& col : colors.log_level_colors) {
			col.reset();
		}
		colors.background.reset();
		colors.foreground.reset();
		colors.auto_complete_selected.reset();
		colors.auto_complete_non_selected.reset();
		colors.auto_complete_separator.reset();
		colors.msg_bg.reset();
	}

	void set_autocomplete_up(bool b) {
		autocomplete_up = b;
	}

private:

	void compute_text_size() noexcept;
	void display_settings_bar() noexcept;
	void display_messages() noexcept;
	void display_command_line() noexcept;

	// displaying command_line itself
	void show_input_text() noexcept;
	void handle_unfocus() noexcept;
	void show_autocomplete() noexcept;

	void call_command() noexcept;
	std::optional<std::string> resolve_history_reference(std::string_view str, bool& modified) const noexcept;
	std::pair<bool, std::string> resolve_history_references(std::string_view str, bool& modified) const;


	static int command_line_callback_st(ImGuiInputTextCallbackData * data) noexcept;
	int command_line_callback(ImGuiInputTextCallbackData* data) noexcept;

	static int try_push_style(ImGuiCol col, const std::optional<ImVec4>& color) {
		if (color) {
			ImGui::PushStyleColor(col, *color);
			return 1;
		}
		return 0;
	}

	////////////
	bool should_show_next_frame{true};
	bool close_request{false};

	const char * const window_name;

	const int base_width;
	const int base_height;

	spdlog::logger local_logger;

	// colors
	struct colors_t {
		std::array<std::optional<ImVec4>, 6> log_level_colors {
				std::optional{ImVec4{0.549f, 0.561f, 0.569f, 1.f}},
				std::optional{ImVec4{0.153f, 0.596f, 0.498f, 1.f}},
				std::optional{ImVec4{0.459f, 0.686f, 0.129f, 1.f}},
				std::optional{ImVec4{0.839f, 0.749f, 0.333f, 1.f}},
				std::optional{ImVec4{1.000f, 0.420f, 0.408f, 1.f}},
				std::optional{ImVec4{1.000f, 0.420f, 0.408f, 1.f}},
		};
		std::optional<ImVec4> background{{0.170f, 0.170f, 0.170f, 1.f}};
		std::optional<ImVec4> foreground{{0.627f, 0.678f, 0.698f, 1.f}};
		std::optional<ImVec4> auto_complete_selected{{1.f, 1.f, 1.f, 1.f}};
		std::optional<ImVec4> auto_complete_non_selected{{0.5f, 0.45f, 0.45f, 1.f}};
		std::optional<ImVec4> auto_complete_separator{{0.6f, 0.6f, 0.6f, 1.f}};
		std::optional<ImVec4> msg_bg{{0.1f, 0.1f, 0.1f, 1.f}};
	};
	colors_t colors{};

	// configuration
	bool autoscroll{true};
	bool autowrap{true};
	std::vector<std::string>::size_type last_size{0u};
	int level{spdlog::level::trace};

	const std::string_view autoscroll_text;
	const std::string_view clear_text;
	const std::string_view log_level_text;
	const std::string_view autowrap_text;

	// messages view variables
	std::string level_list_text{};
	std::optional<ImVec2> selector_size_global{};
	ImVec2 selector_label_size{};

	const std::string* longest_log_level;


	// command line variables
	buffer_type command_buffer{};
	buffer_type::size_type buffer_usage{0u}; // max accessible: command_buffer[buffer_usage - 1] (buffer_usage might be 0 for empty string)
	bool command_entered{false};
	bool should_take_focus{false};

	ImGuiID previously_active_id{0u};
	ImGuiID input_text_id{0u};

	// autocompletion
	std::vector<std::reference_wrapper<const commands::list_element_t>> current_autocomplete{};
	std::string_view autocomlete_separator{"| "};
	bool autocomplete_up{false}; // up or down

	// command line: completion using history
	std::string command_line_backup{};
	std::string_view command_line_backup_prefix{};
	std::vector<std::string> command_history{};
	std::optional<decltype(command_history)::iterator> current_history_selection{};
	
	bool ignore_next_textinput{false};

};

#endif //DUNGEEP_TERMINAL_HPP
