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

	template <typename BidirIt1, typename BidirIt2>
	constexpr void copy_backward(BidirIt1 src_beg, BidirIt1 src_end, BidirIt2 dest_beg, BidirIt2 dest_end) {
		auto copy_length = std::distance(src_beg, src_end);
		auto avail_length = std::distance(dest_beg, dest_end);
		if (avail_length < copy_length) {
			std::advance(src_end, avail_length - copy_length);
		} else {
			std::advance(dest_end, copy_length - avail_length);
		}
		while(src_beg != src_end && dest_beg != dest_end) {
			*--dest_end = *--src_end;
		}
	}

	// Returns new end of dest collection
	// does as if the n first values of dest where non existant
	template <typename ForwardIt, typename RandomAccessIt>
	constexpr RandomAccessIt erase_insert(ForwardIt src_beg, ForwardIt src_end, RandomAccessIt dest_beg, RandomAccessIt dest_end, RandomAccessIt dest_max, unsigned int n) {
		n = std::min(static_cast<unsigned>(std::distance(dest_beg, dest_end)), n);
		auto copy_length = std::distance(src_beg, src_end);
		auto avail_length = std::distance(dest_end, dest_max) + n;

		if (copy_length <= avail_length) {
			misc::copy_backward(dest_beg + n, dest_end, dest_beg + copy_length, dest_end + copy_length);
			misc::copy(src_beg, src_end, dest_beg, dest_beg + copy_length);
			return dest_end + copy_length - n;

		} else {
			std::advance(src_beg, copy_length - avail_length);
			copy_length = avail_length;
			misc::copy_backward(dest_beg + n, dest_end, dest_beg + copy_length, dest_end + copy_length);
			misc::copy(src_beg, src_end, dest_beg, dest_beg + copy_length);
			return dest_end + copy_length - n;
		}
	}

	template <typename ForwardIterator, typename ValueType>
	constexpr ForwardIterator find_last(ForwardIterator begin, ForwardIterator end, const ValueType& val) {
		auto rend = std::reverse_iterator(begin);
		auto rbegin = std::reverse_iterator(end);
		auto search = std::find(rbegin, rend, val);
		if (search == rend) {
			return end;
		}
		return std::prev(search.base());
	}

	constexpr bool is_space(char c) {
		return c == ' ';
	}

	constexpr bool is_digit(char c) {
		return c >= '0' && c <= '9';
	}

	constexpr bool success(std::errc ec) {
		return ec == std::errc{};
	}

	// Returns a vector containing each element that were space separated
	// Returns an empty optional if a '"' char was not matched with a closing '"'
	std::optional<std::vector<std::string>> split_by_space(std::string_view in);

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
			std::string_view::size_type idx = 0;
			while (idx < str.size() && is_space(str[idx])) {
				++idx;
			}
			return (str.size() - idx) >= pr.size() ? str.substr(idx, pr.size()) == pr : false;
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
