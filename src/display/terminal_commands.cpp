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
#include <environment/world.hpp>
#include <sstream>
#include <utils/resource_manager.hpp>
#include <utils/resource_keys.hpp>

using commands::argument;
using commands::list_element_t;

namespace {
	namespace print_resource_args_list {
	}
	std::vector<std::string> print_resource_completion(std::string_view);
}

namespace {
	void clear(const argument&);
	void echo(const argument&);
	void exit(const argument&);
	void help(const argument&);
	void print_resource(const argument&);
	void quit(const argument&);
}

namespace {
	template <typename... Args>
	void print(const argument& arg, Args&&... args) {
		arg.terminal_.command_log().info(std::forward<Args>(args)...);
	}

	template <typename... Args>
	void error(const argument& arg, Args&&... args) {
		arg.terminal_.command_log().error(std::forward<Args>(args)...);
	}
}

namespace commands {

	std::vector<std::string> no_completion(std::string_view);
	void no_command(const argument&);




	// Must be sorted
	constexpr std::array list{
			list_element_t{"clear ", "clears the terminal screen", clear, no_completion},
			list_element_t{"echo ", "prints text", echo, no_completion},
			list_element_t{"exit ", "closes this terminal", exit, no_completion},
			list_element_t{"help ", "show this help", help, no_completion},
			list_element_t{"print ", "prints text", echo, no_completion},
			list_element_t{"print_resource ", "prints resources file", print_resource, print_resource_completion},
			list_element_t{"quit ", "closes this application", quit, no_completion},
	};
	static_assert(misc::is_sorted(list.begin(), list.end()),
	              "commands::list should be lexicographically sorted by command name");

	const list_element_t empty_command{"Empty Command", "Placeholder for unknown command", no_command, no_completion};




	std::vector<std::reference_wrapper<const list_element_t>>
	find_by_prefix(std::string_view prefix) {
		auto compare_name = [](const list_element_t& el) { return el.name; };
		auto map_to_cref = [](const list_element_t& el) { return std::cref(el); };

		return misc::prefix_search(prefix, list.begin(), list.end(), std::move(compare_name), std::move(map_to_cref));
	}


	std::vector<std::string> no_completion(std::string_view) {return {};}
	void no_command(const argument&) {}

}

namespace {
	std::vector<std::string> print_resource_completion(std::string_view command_line) {
		// TODO
		// resources::manager.get_item_count();
		// resources::manager.get_map_list()
//		resources::manager.get_creature_count()
//		resources::manager.get_text(keys::text::lang_name);
//		resources::manager.get_text(keys::text::dflt_lang_name);
		return {};
	}
}

namespace {

	void clear(const argument&) {
		logger::sink->clear();
	}

	void echo(const argument& arg) {
		if (arg.cl_arguments.size() < 2) {
			print(arg, "");
			return;
		}
		if (arg.cl_arguments[1][0] == '-') {
			if (arg.cl_arguments[1] == "--help" || arg.cl_arguments[1] == "-help") {
				print(arg, "usage: {} [text to be printed]", arg.cl_arguments[0]);
			} else {
				error(arg, "Unknown argument: {}", arg.cl_arguments[1]);
			}
		} else {
			std::string str{};
			str = arg.cl_arguments[1];
			for (auto it = std::next(arg.cl_arguments.begin(), 2) ; it != arg.cl_arguments.end() ; ++it) {
				str.reserve(str.size() + it->size() + 1);
				str += ' ';
				str += *it;
			}
			print(arg, "{}", str);
		}
	}

	void exit(const argument& arg) {
		arg.terminal_.set_should_close();
	}

	void help(const argument& arg) {
		constexpr unsigned long list_element_name_max_size = misc::max_size(commands::list.begin(), commands::list.end(),
		                                                                    [](const list_element_t& elem) { return elem.name.size(); });

		print(arg, "Available commands:");
		for (const list_element_t& elem : commands::list) {
			print(arg, "        {:{}}| {}", elem.name, list_element_name_max_size, elem.description);
		}
		print(arg, "");
		print(arg, "Additional information might be available using \"'command' --help\"");
	}

	void print_resource(const argument& arg) {
		error(arg, "TODO");
	}

	void quit(const argument& arg) {
		error(arg, "TODO");
	}
}