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

#include <vector>
#include <string_view>
#include <utils/misc.hpp>

std::optional<std::vector<std::string>> misc::split_by_space(std::string_view in) {
	std::vector<std::string> out;

	auto it = in.begin();
	while (it != in.end() && misc::is_space(*it)) {
		++it;
	}

	if (it == in.end()) {
		return out;
	}

	auto skip_spaces = [&]() {
		do {
			++it;
		} while (it != in.end() && misc::is_space(*it));
	};

	std::string current_string{};
	do {
		if (*it == '"') {
			bool escaped;
			do {
				escaped = (*it == '\\');
				++it;

				if (it != in.end() && (*it != '"' || escaped)) {
					if (*it == '\\') {
						if (escaped) {
							current_string += *it;
						}
					} else {
						current_string += *it;
					}
				} else {
					break;
				}

			} while (true);

			if (it == in.end()) {
				return {};
			}
			++it;

		} else if (misc::is_space(*it)) {
			out.emplace_back(std::move(current_string));
			current_string = {};
			skip_spaces();

		} else if (*it == '\\') {
			++it;
			if (it != in.end()) {
				current_string += *it;
				++it;
			}
		} else {
			current_string += *it;
			++it;
		}

	} while (it != in.end());

	if (!current_string.empty()) {
		out.emplace_back(std::move(current_string));
	}

	return out;
}
