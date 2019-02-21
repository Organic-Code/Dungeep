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


namespace commands {

	std::vector<std::string> no_completion(std::string_view);

	void help(argument&);

	void clear(argument&);

	// Must be sorted
	static constexpr std::array list{
			list_element_t{"clear", "clears the terminal screen", clear, no_completion},
			list_element_t{"help", "show this help", help, no_completion},
	};

	static_assert(misc::is_sorted(list.begin(), list.end()),
	              "commands::list should be lexicographically sorted by command name");

	std::vector<std::string> no_completion(std::string_view) {
		return {};
	}

	void help(argument& arg) {
		arg.terminal_.command_log().info("Available commands:");
		for (const list_element_t& elem : commands::list) {
			arg.terminal_.command_log().info("        {} - {}", elem.name, elem.description);
		}
	}

	void clear(argument&) {
		logger::sink->clear();
	}

}