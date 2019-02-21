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
	void no_command(const argument&);

	void help(const argument&);

	void clear(const argument&);




	// Must be sorted
	constexpr std::array<list_element_t, 2> list{
			list_element_t{"clear ", "clears the terminal screen", clear, no_completion},
			list_element_t{"help ", "show this help", help, no_completion},
	};
	static_assert(misc::is_sorted(list.begin(), list.end()),
	              "commands::list should be lexicographically sorted by command name");

	constexpr unsigned long list_element_name_max_size = misc::max_size(list.begin(), list.end(), [](const list_element_t& elem) { return elem.name.size(); });

	const list_element_t empty_command{"Empty Command", "Placeholder for unknown command", no_command, no_completion};




	std::vector<std::reference_wrapper<const list_element_t>>
	find_by_prefix(terminal::buffer_type::const_iterator prefix_begin, terminal::buffer_type::const_iterator prefix_end) {
		std::vector<std::reference_wrapper<const list_element_t>> ans;

		unsigned int list_idx = 0u;
		unsigned int char_idx = 0u;

		for (auto it = prefix_begin ; it != prefix_end ; ++it) {
			while (list_idx < list.size() && list[list_idx].name[char_idx] < *it) {
				++list_idx;
			}
			++char_idx;

			if (list_idx == list.size() || list[list_idx].name.substr(0, char_idx) != std::string_view(&*prefix_begin, char_idx)) {
				return ans;
			}
		}
		do {
			ans.emplace_back(list[list_idx++]);
		} while(list_idx < list.size() && list[list_idx].name.substr(0, char_idx) == std::string_view(&*prefix_begin, char_idx));

		return ans;
	}


	std::vector<std::string> no_completion(std::string_view) {return {};}
	void no_command(const argument&) {}

	void help(const argument& arg) {
		arg.terminal_.command_log().info("Available commands:");
		for (const list_element_t& elem : commands::list) {
			arg.terminal_.command_log().info("        {:{}}| {}", elem.name, list_element_name_max_size, elem.description);
		}
		arg.terminal_.command_log().info("");
		arg.terminal_.command_log().info("Additional information might be available using \"'command name' help\"");
	}

	void clear(const argument&) {
		logger::sink->clear();
	}

}