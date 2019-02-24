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
			terminal_commands::command_type{"clear ", "clears the terminal screen", terminal_commands::clear, terminal_commands::no_completion},
			terminal_commands::command_type{"configure_terminal ", "configures terminal behaviour and appearance", terminal_commands::configure_term, terminal_commands::no_completion},
			terminal_commands::command_type{"echo ", "prints text", terminal_commands::echo, terminal_commands::no_completion},
			terminal_commands::command_type{"exit ", "closes this terminal", terminal_commands::exit, terminal_commands::no_completion},
			terminal_commands::command_type{"help ", "show this help", terminal_commands::help, terminal_commands::no_completion},
			terminal_commands::command_type{"print ", "prints text", terminal_commands::echo, terminal_commands::no_completion},
			terminal_commands::command_type{"print_resource ", "prints resources file", terminal_commands::print_resource, terminal_commands::no_completion},
			terminal_commands::command_type{"quit ", "closes this application", terminal_commands::quit, terminal_commands::no_completion},
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
	if (arg.command_line.size() <3) {
		return;
	}
	if (arg.command_line[1] == "completion") {
		if (arg.command_line[2] == "set_up") {
			arg.term.set_autocomplete_up(true);
		} else if (arg.command_line[2] == "set_down") {
			arg.term.set_autocomplete_up(false);
		}
	} else if (arg.command_line[1] == "colors") {
		if (arg.command_line[2] == "reset") {
			arg.term.reset_colors();
		}
	}
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