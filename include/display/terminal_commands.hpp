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

#ifndef DUNGEEP_TERMINAL_COMMANDS_HPP
#define DUNGEEP_TERMINAL_COMMANDS_HPP

#include <array>
#include <string>
#include <vector>

#include <display/terminal.hpp>

class world;

namespace commands {
	struct argument {
		world& world_;
		terminal& terminal_;

		std::vector<std::string> cl_arguments;
	};

	struct list_element_t {
		using command_function = void (*)(const argument&);
		using further_completion_function = std::vector<std::string> (*) (std::string_view argument_line);

		const std::string_view name;
		const std::string_view description;
		const command_function call;
		const further_completion_function complete;

		constexpr bool operator<(std::string_view str) const {
			return name < str;
		}

		constexpr bool operator<(const list_element_t& el) const {
			return name < el.name;
		}
	};

	extern const list_element_t empty_command;

	// todo: passer par référence ?
	std::vector<std::reference_wrapper<const list_element_t>> find_by_prefix(std::string_view prefix);

	inline std::vector<std::reference_wrapper<const list_element_t>>
	find_by_prefix(terminal::buffer_type::const_iterator p_beg, terminal::buffer_type::const_iterator p_end) {
		return find_by_prefix({p_beg, static_cast<unsigned long>(p_end - p_beg)});
	}

	inline std::vector<std::reference_wrapper<const list_element_t>> list_commands() {
		return find_by_prefix(std::string_view{});
	}

}

#endif //DUNGEEP_TERMINAL_COMMANDS_HPP
