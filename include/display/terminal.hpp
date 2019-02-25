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
		using further_completion_function = std::vector<std::string> (*)(argument_t<Terminal>& argument_line);

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

	struct theme {
		struct constexpr_color {
			float r,g,b,a;

			ImVec4 imv4() const {
				return {r,g,b,a};
			}
		};

		std::string_view name;

		std::optional<constexpr_color> text;
		std::optional<constexpr_color> window_bg;
		std::optional<constexpr_color> border;
		std::optional<constexpr_color> border_shadow;
		std::optional<constexpr_color> button;
		std::optional<constexpr_color> button_hovered;
		std::optional<constexpr_color> button_active;
		std::optional<constexpr_color> frame_bg;
		std::optional<constexpr_color> frame_bg_hovered;
		std::optional<constexpr_color> frame_bg_active;
		std::optional<constexpr_color> text_selected_bg;
		std::optional<constexpr_color> check_mark;
		std::optional<constexpr_color> title_bg;
		std::optional<constexpr_color> title_bg_active;
		std::optional<constexpr_color> title_bg_collapsed;
		std::optional<constexpr_color> message_panel;
		std::optional<constexpr_color> auto_complete_selected;
		std::optional<constexpr_color> auto_complete_non_selected;
		std::optional<constexpr_color> auto_complete_separator;
		std::optional<constexpr_color> cmd_backlog;
		std::optional<constexpr_color> cmd_history_completed;
		std::optional<constexpr_color> log_level_drop_down_list_bg;
		std::optional<constexpr_color> log_level_active;
		std::optional<constexpr_color> log_level_hovered;
		std::optional<constexpr_color> log_level_selected;
		std::optional<constexpr_color> scrollbar_bg;
		std::optional<constexpr_color> scrollbar_grab;
		std::optional<constexpr_color> scrollbar_grab_active;
		std::optional<constexpr_color> scrollbar_grab_hovered;

		std::array<std::optional<constexpr_color>, 6> log_level_colors{};
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

		theme& theme() {
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

		static int try_push_style(ImGuiCol col, const std::optional<theme::constexpr_color>& color) {
			if (color) {
				ImGui::PushStyleColor(col, color->imv4());
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

		struct theme m_colors{};

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


	namespace themes {

		constexpr theme light = {
				"Light Rainbow",
				theme::constexpr_color{0.100f, 0.100f, 0.100f, 1.000f}, //text
				theme::constexpr_color{0.243f, 0.443f, 0.624f, 1.000f}, //window_bg
				theme::constexpr_color{0.600f, 0.600f, 0.600f, 1.000f}, //border
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //border_shadow
				theme::constexpr_color{0.902f, 0.843f, 0.843f, 0.875f}, //button
				theme::constexpr_color{0.824f, 0.765f, 0.765f, 0.875f}, //button_hovered
				theme::constexpr_color{0.627f, 0.569f, 0.569f, 0.875f}, //button_active
				theme::constexpr_color{0.902f, 0.843f, 0.843f, 0.875f}, //frame_bg
				theme::constexpr_color{0.824f, 0.765f, 0.765f, 0.875f}, //frame_bg_hovered
				theme::constexpr_color{0.627f, 0.569f, 0.569f, 0.875f}, //frame_bg_active
				theme::constexpr_color{0.260f, 0.590f, 0.980f, 0.350f}, //text_selected_bg
				theme::constexpr_color{0.843f, 0.000f, 0.373f, 1.000f}, //check_mark
				theme::constexpr_color{0.243f, 0.443f, 0.624f, 0.850f}, //title_bg
				theme::constexpr_color{0.165f, 0.365f, 0.506f, 1.000f}, //title_bg_active
				theme::constexpr_color{0.243f, 0.443f, 0.624f, 0.850f}, //title_collapsed
				theme::constexpr_color{0.902f, 0.843f, 0.843f, 0.875f}, //message_panel
				theme::constexpr_color{0.196f, 1.000f, 0.196f, 1.000f}, //auto_complete_selected
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 1.000f}, //auto_complete_non_selected
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.392f}, //auto_complete_separator
				theme::constexpr_color{0.519f, 0.118f, 0.715f, 1.000f}, //cmd_backlog
				theme::constexpr_color{1.000f, 0.430f, 0.059f, 1.000f}, //cmd_history_completed
				theme::constexpr_color{0.901f, 0.843f, 0.843f, 0.784f}, //log_level_drop_down_list_bg
				theme::constexpr_color{0.443f, 0.705f, 1.000f, 1.000f}, //log_level_active
				theme::constexpr_color{0.443f, 0.705f, 0.784f, 0.705f}, //log_level_hovered
				theme::constexpr_color{0.443f, 0.623f, 0.949f, 1.000f}, //log_level_selected
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //scrollbar_bg
				theme::constexpr_color{0.470f, 0.470f, 0.588f, 1.000f}, //scrollbar_grab
				theme::constexpr_color{0.392f, 0.392f, 0.509f, 1.000f}, //scrollbar_grab_active
				theme::constexpr_color{0.509f, 0.509f, 0.666f, 1.000f}, //scrollbar_grab_hovered
				{
					theme::constexpr_color{0.078f, 0.117f, 0.764f, 1.f}, // log_level::trace
					{}, // log_level::debug
					theme::constexpr_color{0.301f, 0.529f, 0.000f, 1.f}, // log_level::info
					theme::constexpr_color{0.784f, 0.431f, 0.058f, 1.f}, // log_level::warning
					theme::constexpr_color{0.901f, 0.117f, 0.117f, 1.f}, // log_level::error
					theme::constexpr_color{0.901f, 0.117f, 0.117f, 1.f}, // log_level::critical
				}
		};

		constexpr theme cherry {
			"Dark Cherry",
			theme::constexpr_color{0.649f, 0.661f, 0.669f, 1.000f}, //text
			theme::constexpr_color{0.130f, 0.140f, 0.170f, 1.000f}, //window_bg
			theme::constexpr_color{0.310f, 0.310f, 1.000f, 0.000f}, //border
			theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //border_shadow
			theme::constexpr_color{0.470f, 0.770f, 0.830f, 0.140f}, //button
			theme::constexpr_color{0.455f, 0.198f, 0.301f, 0.860f}, //button_hovered
			theme::constexpr_color{0.455f, 0.198f, 0.301f, 1.000f}, //button_active
			theme::constexpr_color{0.200f, 0.220f, 0.270f, 1.000f}, //frame_bg
			theme::constexpr_color{0.455f, 0.198f, 0.301f, 0.780f}, //frame_bg_hovered
			theme::constexpr_color{0.455f, 0.198f, 0.301f, 1.000f}, //frame_bg_active
			theme::constexpr_color{0.455f, 0.198f, 0.301f, 0.430f}, //text_selected_bg
			theme::constexpr_color{0.710f, 0.202f, 0.207f, 1.000f}, //check_mark
			theme::constexpr_color{0.232f, 0.201f, 0.271f, 1.000f}, //title_bg
			theme::constexpr_color{0.502f, 0.075f, 0.256f, 1.000f}, //title_bg_active
			theme::constexpr_color{0.200f, 0.220f, 0.270f, 0.750f}, //title_bg_collapsed
			theme::constexpr_color{0.100f, 0.100f, 0.100f, 0.500f}, //message_panel
			theme::constexpr_color{1.000f, 1.000f, 1.000f, 1.000f}, //auto_complete_selected
			theme::constexpr_color{0.500f, 0.450f, 0.450f, 1.000f}, //auto_complete_non_selected
			theme::constexpr_color{0.600f, 0.600f, 0.600f, 1.000f}, //auto_complete_separator
			theme::constexpr_color{0.860f, 0.930f, 0.890f, 1.000f}, //cmd_backlog
			theme::constexpr_color{0.153f, 0.596f, 0.498f, 1.000f}, //cmd_history_completed
			theme::constexpr_color{0.100f, 0.100f, 0.100f, 0.860f}, //log_level_drop_down_list_bg
			theme::constexpr_color{0.730f, 0.130f, 0.370f, 1.000f}, //log_level_active
			theme::constexpr_color{0.450f, 0.190f, 0.300f, 0.430f}, //log_level_hovered
			theme::constexpr_color{0.730f, 0.130f, 0.370f, 0.580f}, //log_level_selected
			theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //scrollbar_bg
			theme::constexpr_color{0.690f, 0.690f, 0.690f, 0.800f}, //scrollbar_grab
			theme::constexpr_color{0.490f, 0.490f, 0.490f, 0.800f}, //scrollbar_grab_active
			theme::constexpr_color{0.490f, 0.490f, 0.490f, 1.000f}, //scrollbar_grab_hovered
			{
				theme::constexpr_color{0.549f, 0.561f, 0.569f, 1.f}, // log_level::trace
				theme::constexpr_color{0.153f, 0.596f, 0.498f, 1.f}, // log_level::debug
				theme::constexpr_color{0.459f, 0.686f, 0.129f, 1.f}, // log_level::info
				theme::constexpr_color{0.839f, 0.749f, 0.333f, 1.f}, // log_level::warning
				theme::constexpr_color{1.000f, 0.420f, 0.408f, 1.f}, // log_level::error
				theme::constexpr_color{1.000f, 0.420f, 0.408f, 1.f}, // log_level::critical
			},
		};

		constexpr std::array list {
				cherry,
				light
		};
	}
}

#include "terminal.tpp"

#undef TERM_STATIC_ASSERT_TERMINALHELPER_WELLFORMED

#endif //DUNGEEP_TERMINAL_HPP
