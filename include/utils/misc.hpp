#ifndef DUNGEEP_MISC_HPP
#define DUNGEEP_MISC_HPP

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



#include <utility>
#include <iterator>
#include <functional>
#include <type_traits>

namespace misc {

	// std::is_sorted is not constexpr pre C++20
	template <typename ForwardIt, typename EndIt, typename Comparator = std::less<std::remove_reference_t<decltype(*std::declval<ForwardIt>())>>>
	constexpr bool is_sorted(ForwardIt it, EndIt last, Comparator&& comp = {}) {
		if (it == last) {
			return true;
		}

		for (ForwardIt next = std::next(it) ; next != last ; ++next, ++it) {
			if (comp(*next, *it)) {
				return false;
			}
		}
		return true;
	}
}

#endif //DUNGEEP_MISC_HPP