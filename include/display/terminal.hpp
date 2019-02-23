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

	terminal();

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
	std::optional<std::string_view> resolve_history_reference(std::string_view str, bool& modified) const noexcept;
	std::pair<bool, std::string> resolve_history_references(std::string_view str, bool& modified) const;


	static int command_line_callback_st(ImGuiInputTextCallbackData * data) noexcept;
	int command_line_callback(ImGuiInputTextCallbackData* data) noexcept;

	////////////
	bool should_show_next_frame{true};
	bool close_request{false};

	spdlog::logger local_logger;

	// configuration
	bool autoscroll{true};
	std::vector<std::string>::size_type last_size{0u};
	int level{spdlog::level::trace};


	// messages view variables
	const std::string_view autoscroll_text;
	const std::string_view clear_text;
	const std::string_view log_level_text;
	std::string level_list_text{};
	std::optional<ImVec2> selector_size_global{};
	ImVec2 selector_label_size{};

	const std::string* longest_log_level;


	// command line variables
	buffer_type command_buffer{};
	buffer_type::size_type buffer_usage{0u}; // max accessible: command_buffer[buffer_usage - 1] (buffer_usage might be 0 for empty string)
	std::vector<std::reference_wrapper<const commands::list_element_t>> current_autocomplete{};
	bool command_entered{false};
	bool should_take_focus{false};

	std::string_view autocomlete_separator{"| "};

	ImGuiID previously_active_id{0u};
	ImGuiID input_text_id{0u};

	// command line: completion using history
	std::string command_line_backup{};
	std::string_view command_line_backup_prefix{};
	std::vector<std::string> command_history{};
	std::optional<decltype(command_history)::iterator> current_history_selection{};
	
	bool ignore_next_textinput{false};

};

#endif //DUNGEEP_TERMINAL_HPP
