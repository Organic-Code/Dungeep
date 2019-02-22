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

	// std::identity is c++20
	struct identity {
		template<typename T>
		constexpr auto operator()(T&& t) const {
			return std::forward<T>(t);
		}
	};

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

	template <typename ForwardIt, typename EndIt, typename SizeExtractor>
	constexpr auto max_size(ForwardIt it, EndIt last, SizeExtractor&& size_of) -> decltype(size_of(*it)){
		using size_type = decltype(size_of(*it));
		size_type max_size = 0u;
		while (it != last) {
			size_type current_size = size_of(*it++);
			if (current_size > max_size) {
				max_size = current_size;
			}
		}
		return max_size;
	}

	template <typename ForwardIt, typename OutputIt>
	constexpr void copy(ForwardIt src_beg, ForwardIt src_end, OutputIt dest_beg, OutputIt dest_end) {
		while(src_beg != src_end && dest_beg != dest_end) {
			*dest_beg++ = *src_beg++;
		}
	}

	constexpr bool is_space(char c) {
		return c == ' ';
	}

	std::vector<std::string_view> split_by_space(std::string_view in);

	/// Search any element matching "prefix" in the collection formed by [c_beg, c_end)
	/// str_ext must map types *ForwardIt() to std::string_view
	/// transform is whatever transformation you want to do to the matching elements.
	template <typename ForwardIt, typename StrExtractor = identity, typename Transform = identity>
	auto prefix_search(std::string_view prefix, ForwardIt c_beg, ForwardIt c_end,
			StrExtractor&& str_ext = {}, Transform&& transform = {}) -> std::vector<decltype(transform(*c_beg))> {

		auto lower = std::lower_bound(c_beg, c_end, prefix);
		auto higher = std::upper_bound(lower, c_end, prefix, [&str_ext](std::string_view pre, const auto& element) {
			std::string_view val = str_ext(element);
			return pre.substr(0, std::min(val.size(), pre.size())) < val.substr(0, std::min(val.size(), pre.size()));
		});

		std::vector<decltype(transform(*c_beg))> ans;
		std::transform(lower, higher, std::back_inserter(ans), std::forward<Transform>(transform));
		return ans;
	}

	template <typename ForwardIt, typename StrExtractor = identity>
	ForwardIt find_first_prefixed(std::string_view prefix, ForwardIt beg, ForwardIt end, StrExtractor&& str_ext = {}) {
		// std::string_view::start_with is C++20
		auto start_with = [](std::string_view str, std::string_view pr) {
			return str.size() >= pr.size() ? str.substr(0, pr.size()) == pr : false;
		};
		while (beg != end) {
			if (start_with(str_ext(*beg), prefix)) {
				return beg;
			}
			++beg;
		}
		return beg;
	}

}

#endif //DUNGEEP_MISC_HPP
