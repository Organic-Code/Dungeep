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

#include <display/terminal_commands.hpp>
#include <utils/misc.hpp>
#include <utils/logger.hpp>
#include <display/terminal.hpp>

namespace {

	constexpr std::array local_command_list {
			terminal_commands::command_type{"clear", "clears the terminal screen", terminal_commands::clear, terminal_commands::no_completion},
			terminal_commands::command_type{"configure_terminal", "configures terminal behaviour and appearance", terminal_commands::configure_term, terminal_commands::configure_term_autocomplete},
			terminal_commands::command_type{"echo", "prints text", terminal_commands::echo, terminal_commands::no_completion},
			terminal_commands::command_type{"exit", "closes this terminal", terminal_commands::exit, terminal_commands::no_completion},
			terminal_commands::command_type{"help", "show this help", terminal_commands::help, terminal_commands::no_completion},
			terminal_commands::command_type{"print", "prints text", terminal_commands::echo, terminal_commands::no_completion},
			terminal_commands::command_type{"print_resource", "prints resources file", terminal_commands::print_resource, terminal_commands::no_completion},
			terminal_commands::command_type{"quit", "closes this application", terminal_commands::quit, terminal_commands::no_completion},
	};
}

terminal_commands::terminal_commands() {
	for (const command_type& cmd : local_command_list) {
		add_command_(cmd);
	}
}

void terminal_commands::clear(argument_type&) {
	logger::sink->clear();
}

void terminal_commands::configure_term(argument_type& arg) {
	auto parse = [](const std::string& str, auto& val) -> bool {
		std::from_chars_result fcr = std::from_chars(str.data(), str.data() + str.size(), val, 10);
		return misc::success(fcr.ec) && fcr.ptr == str.data() + str.size();
	};

	std::vector<std::string>& cl = arg.command_line;
	if (cl.size() < 3) {
		return;
	}
	if (cl[1] == "completion") {
		if (cl[2] == "up") {
			arg.term.set_autocomplete_pos(term_t::position::up);
		} else if (cl[2] == "down") {
			arg.term.set_autocomplete_pos(term_t::position::down);
		} else if (cl[2] == "disable") {
			arg.term.set_autocomplete_pos(term_t::position::nowhere);
		}
	} else if (cl[1] == "colors") {
		if (cl[2] == "reset") {
			arg.term.reset_colors();
		} else if (cl[2] == "set_theme" && (cl.size() == 7 || cl.size() == 8)) {

			unsigned int col_idx;
			std::uint8_t r, g, b;
			std::uint8_t a = 255;
			if (!parse(cl[3], col_idx) || !parse(cl[4], r) || !parse(cl[5], g) || !parse(cl[6], b)) {
				return;
			}
			if (cl.size() == 8) {
				if (!parse(cl[7], a)) {
					return;
				}
			}
			if (col_idx >= arg.term.colors().theme_colors.size()) {
				return;
			}
			arg.term.colors().theme_colors[col_idx] = ImVec4{
				static_cast<float>(r) / 255.f,
				static_cast<float>(g) / 255.f,
				static_cast<float>(b) / 255.f,
				static_cast<float>(a) / 255.f
			};
		} else if (cl[2] == "reset_theme" && cl.size() < 5) {
			if (cl.size() == 4) {
				std::uint8_t col_idx;
				if (parse(cl[3], col_idx) && col_idx < arg.term.colors().theme_colors.size()) {
					arg.term.colors().theme_colors[col_idx].reset();
				}
			} else {
				for (std::optional<ImVec4>& color : arg.term.colors().theme_colors) {
					color.reset();
				}
			}
		}
	}
}

std::vector<std::string> terminal_commands::configure_term_autocomplete(const std::vector<std::string>& args) {
	std::vector<std::string> ans;
	auto try_match = [&ans](std::string_view vs, const std::string& str) {
		if (vs.substr(0, std::min(vs.size(), str.size())) == str) {
			ans.emplace_back(vs.data(), vs.size());
		}
	};

	if (args.size() == 2) {
		try_match("completion", args[1]);
		try_match("colors", args[1]);
	} else if (args.size() == 3) {
		if (args[1] == "completion") {
			try_match("down", args[2]);
			try_match("disable", args[2]);
			try_match("up", args[2]);

		} else if (args[1] == "colors") {
			try_match("reset", args[2]);
			try_match("reset_theme", args[2]);
			try_match("set_theme", args[2]);

		}
	}
	return ans;
}

void terminal_commands::echo(argument_type& arg) {
	if (arg.command_line.size() < 2) {
		arg.term.print("");
		return;
	}
	if (arg.command_line[1][0] == '-') {
		if (arg.command_line[1] == "--help" || arg.command_line[1] == "-help") {
			arg.term.print("usage: {} [text to be printed]", arg.command_line[0]);
		} else {
			arg.term.print_error("Unknown argument: {}", arg.command_line[1]);
		}
	} else {
		std::string str{};
		auto it = std::next(arg.command_line.begin(), 1);
		while (it != arg.command_line.end() && it->empty()) {
			++it;
		}
		if (it != arg.command_line.end()) {
			str = *it;
			for (++it ; it != arg.command_line.end() ; ++it) {
				if (it->empty()) {
					continue;
				}
				str.reserve(str.size() + it->size() + 1);
				str += ' ';
				str += *it;
			}
		}
		arg.term.print("{}", str);
	}
}

void terminal_commands::exit(argument_type& arg) {
	arg.term.set_should_close();
}

void terminal_commands::help(argument_type& arg) {
	constexpr unsigned long list_element_name_max_size = misc::max_size(local_command_list.begin(), local_command_list.end(),
			[](const command_type& cmd) { return cmd.name.size(); });

	arg.term.print("Available commands:");
	for (const command_type& cmd : local_command_list) {
		arg.term.print("        {:{}}| {}", cmd.name, list_element_name_max_size, cmd.description);
	}
	arg.term.print("");
	arg.term.print("Additional information might be available using \"'command' --help\"");
}

void terminal_commands::print_resource(argument_type& arg) {
	arg.term.print_error("TODO");
}

void terminal_commands::quit(argument_type& arg) {
	arg.term.print_error("TODO");
}
