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
#include <utility>
#include <spdlog/spdlog.h>
#include <utils/misc.hpp>

#include <imgui.h>

#define TERM_STATIC_ASSERT_TPARAM_WELLFORMED(tparam)                                                                                     \
template<typename T>                                                                                                                     \
using find_commands_by_prefix_method_v1 =                                                                                                \
decltype(std::declval<T&>().find_commands_by_prefix(std::declval<std::string_view>()));                                                  \
template<typename T>                                                                                                                     \
using find_commands_by_prefix_method_v2 =                                                                                                \
decltype(std::declval<T&>().find_commands_by_prefix(std::declval<const char *>(), std::declval<const char *>()));                        \
template<typename T>                                                                                                                     \
using list_commands_method = decltype(std::declval<T&>().list_commands());                                                               \
static_assert(                                                                                                                           \
		misc::is_detected_with_return_type_v<find_commands_by_prefix_method_v1, std::vector<command_type_cref>, tparam>,                 \
		"TerminalHelper should implement the method 'std::vector<command_type_cref> find_command_by_prefix(std::string_view)'. "         \
		"See term::basic_terminal_helper for reference");                                                                                \
static_assert(                                                                                                                           \
		misc::is_detected_with_return_type_v<find_commands_by_prefix_method_v2, std::vector<command_type_cref>, tparam>,                 \
		"TerminalHelper should implement the method 'std::vector<command_type_cref> find_command_by_prefix(const char*, const char*)'. " \
		"See term::basic_terminal_helper for reference");                                                                                \
static_assert(                                                                                                                           \
		misc::is_detected_with_return_type_v<list_commands_method, std::vector<command_type_cref>, tparam>,                              \
		"TerminalHelper should implement the method 'std::vector<command_type_cref> list_commands()'. "                                  \
		"See term::basic_terminal_helper for reference")


namespace term {

	template<typename Terminal>
	struct argument_t {
		typename Terminal::value_type& val;
		Terminal& term;

		std::vector<std::string> command_line;
	};

	template<typename Terminal>
	struct command_t {
		using command_function = void (*)(argument_t<Terminal>&);
		using further_completion_function = std::vector<std::string> (*)(const std::vector<std::string>& argument_line);

		std::string_view name{};
		std::string_view description{};
		command_function call{};
		further_completion_function complete{};

		friend constexpr bool operator<(const command_t& lhs, const command_t& rhs) {
			return lhs.name < rhs.name;
		}

		friend constexpr bool operator<(const command_t& lhs, const std::string_view& rhs) {
			return lhs.name < rhs;
		}

		friend constexpr bool operator<(const std::string_view& lhs, const command_t& rhs) {
			return lhs < rhs.name;
		}
	};

	struct colors_t {

		std::optional<ImVec4>& operator[](ImGuiCol c) {
			return theme_colors[c];
		}

		const std::optional<ImVec4>& operator[](ImGuiCol c) const {
			return theme_colors[c];
		}

		std::array<std::optional<ImVec4>, 6> log_level_colors{
				std::optional{ImVec4{0.549f, 0.561f, 0.569f, 1.f}},
				std::optional{ImVec4{0.153f, 0.596f, 0.498f, 1.f}},
				std::optional{ImVec4{0.459f, 0.686f, 0.129f, 1.f}},
				std::optional{ImVec4{0.839f, 0.749f, 0.333f, 1.f}},
				std::optional{ImVec4{1.000f, 0.420f, 0.408f, 1.f}},
				std::optional{ImVec4{1.000f, 0.420f, 0.408f, 1.f}},
		};
		std::optional<ImVec4> auto_complete_selected{{1.f, 1.f, 1.f, 1.f}};
		std::optional<ImVec4> auto_complete_non_selected{{0.5f, 0.45f, 0.45f, 1.f}};
		std::optional<ImVec4> auto_complete_separator{{0.6f, 0.6f, 0.6f, 1.f}};
		std::optional<ImVec4> message_panel{{0.1f, 0.1f, 0.1f, 0.5f}};

		std::array<std::optional<ImVec4>, ImGuiCol_COUNT> theme_colors;

	};

	template<typename TerminalHelper>
	class terminal {
	public:
		enum class position {
			up,
			down,
			nowhere // disabled
		};

		using buffer_type = std::array<char, 1024>;
		using value_type = typename TerminalHelper::value_type;
		using command_type = command_t<terminal<TerminalHelper>>;
		using command_type_cref = std::reference_wrapper<const command_type>;
		using argument_type = argument_t<terminal>;

		TERM_STATIC_ASSERT_TPARAM_WELLFORMED(TerminalHelper);

		explicit terminal(value_type& arg_value, const char * window_name_ = "terminal", int base_width_ = 900,
		                  int base_height_ = 200, TerminalHelper&& th = {});

		bool show() noexcept;

		void hide() noexcept {
			m_previously_active_id = 0;
		}

		const std::vector<std::string>& get_history() const noexcept {
			return m_command_history;
		}

		void set_should_close() noexcept {
			m_close_request = true;
		}

		void reset_colors() noexcept;

		colors_t& colors() {
			return m_colors;
		}

		void set_autocomplete_pos(position p) {
			m_autocomplete_pos = p;
		}

		position get_autocomplete_pos() const {
			return m_autocomplete_pos;
		}

		template<typename... Args>
		void print(const char *fmt, Args&&... args) {
			m_local_logger.info(fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		void print_error(const char *fmt, Args&&... args) {
			m_local_logger.error(fmt, std::forward<Args>(args)...);
		}

		void clear();

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

		int command_line_callback(ImGuiInputTextCallbackData * data) noexcept;

		static int try_push_style(ImGuiCol col, const std::optional<ImVec4>& color) {
			if (color) {
				ImGui::PushStyleColor(col, *color);
				return 1;
			}
			return 0;
		}


		int is_space(std::string_view str) const;

		bool is_digit(char c) const;

		unsigned long get_length(std::string_view str) const;


		// Returns a vector containing each element that were space separated
		// Returns an empty optional if a '"' char was not matched with a closing '"',
		//                except if ignore_non_match was set to true
		std::optional<std::vector<std::string>> split_by_space(std::string_view in, bool ignore_non_match = false) const;

		////////////
		value_type& m_argument_value;
		mutable TerminalHelper m_t_helper;

		bool m_should_show_next_frame{true};
		bool m_close_request{false};

		const char * const m_window_name;

		const int m_base_width;
		const int m_base_height;

		spdlog::logger m_local_logger;

		colors_t m_colors{};

		// configuration
		bool m_autoscroll{true};
		bool m_autowrap{true};
		std::vector<std::string>::size_type m_last_size{0u};
		int m_level{spdlog::level::trace};

		const std::string_view m_autoscroll_text;
		const std::string_view m_clear_text;
		const std::string_view m_log_level_text;
		const std::string_view m_autowrap_text;

		// messages view variables
		std::string m_level_list_text{};
		std::optional<ImVec2> m_selector_size_global{};
		ImVec2 m_selector_label_size{};

		const std::string * m_longest_log_level;


		// command line variables
		buffer_type m_command_buffer{};
		buffer_type::size_type m_buffer_usage{0u}; // max accessible: command_buffer[buffer_usage - 1]
		                                           // (buffer_usage might be 0 for empty string)
		buffer_type::size_type m_previous_buffer_usage{0u};
		bool m_should_take_focus{false};

		ImGuiID m_previously_active_id{0u};
		ImGuiID m_input_text_id{0u};

		// autocompletion
		std::vector<command_type_cref> m_current_autocomplete{};
		std::vector<std::string> m_current_autocomplete_strings{};
		std::string_view m_autocomlete_separator{" | "};
		position m_autocomplete_pos{position::down};
		bool m_command_entered{false};

		// command line: completion using history
		std::string m_command_line_backup{};
		std::string_view m_command_line_backup_prefix{};
		std::vector<std::string> m_command_history{};
		std::optional<std::vector<std::string>::iterator> m_current_history_selection{};
		unsigned long m_last_flush_at_history{0u}; // for the [-n] indicator on command line
		bool m_flush_bit{false};

		bool m_ignore_next_textinput{false};
		bool m_has_focus{false};

	};
}

#include "terminal.tpp"

#undef TERM_STATIC_ASSERT_TERMINALHELPER_WELLFORMED

#endif //DUNGEEP_TERMINAL_HPP
