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

class terminal {
public:
	using buffer_type = std::array<char, 1024>;

	terminal();

	void show();

	const std::vector<std::string>& get_history() const {
		return command_history;
	}

	spdlog::logger& command_log() {
		return local_logger;
	}

private:

	void compute_text_size();
	void display_settings_bar();
	void display_messages();
	void display_command_line();

	static int command_line_callback(ImGuiInputTextCallbackData* data) noexcept;

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
	std::vector<std::string> command_history{};
	buffer_type command_buffer{};
	buffer_type::size_type buffer_usage{0u};
	std::vector<std::string> current_autocomplete{};

	spdlog::logger local_logger;

	ImGuiID previously_active_id{0u};
};

#endif //DUNGEEP_TERMINAL_HPP