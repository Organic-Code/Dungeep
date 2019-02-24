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

#include <environment/world.hpp>
#include <display/terminal.hpp>
#include <display/demo_terminal_helpers.hpp>

class terminal_commands : public term::basic_terminal_helper<terminal_commands, world> {
public:

	terminal_commands();

	static std::vector<std::string> no_completion(std::string_view) { return {}; };

	static void clear(argument_type&);
	static void configure_term(argument_type&);
	static void echo(argument_type&);
	static void exit(argument_type&);
	static void help(argument_type&);
	static void print_resource(argument_type&);
	static void quit(argument_type&);
};


#endif //DUNGEEP_TERMINAL_COMMANDS_HPP
