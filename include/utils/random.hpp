#ifndef DUNGEEP_RANDOM_HPP
#define DUNGEEP_RANDOM_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018, Lucas Lazare                                                                                                     ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,         ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  is furnished to do so, subject to the following conditions:                                                                        ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders X be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,      ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <random>
#include <type_traits>
#include <iostream>

namespace dungeep {

	inline std::mt19937_64 random_engine(std::random_device{}());

	// std::normal_distribution is not portable across compilers
	template <typename RealType = double>
	class normal_distribution {
		static_assert(std::is_floating_point_v<RealType>, "result_type must be a floating point type");
	public:
		using result_type = RealType;
		struct param_type {
			using distribution_type = normal_distribution<RealType>;

			explicit constexpr param_type(RealType avg_ = 0, RealType stddev_ = 1) noexcept
				: m_avg(avg_), m_stddev(stddev_) {}

			constexpr RealType mean() const noexcept {
				return m_avg;
			}

			constexpr RealType stddev() const noexcept {
				return m_stddev;
			}

			constexpr bool operator==(const param_type& other) const noexcept {
				return m_avg == other.m_avg && m_stddev == other.stddev;
			}

			constexpr bool operator!=(const param_type& other) const noexcept {
				return !(*this == other);
			}


		private:
			RealType m_avg, m_stddev;
		};


		constexpr normal_distribution() noexcept
			: normal_distribution(0.0) {}
		explicit constexpr normal_distribution(RealType mean, RealType stddev = 1.0) noexcept
			: normal_distribution(param_type(mean, stddev)) {}
		explicit constexpr normal_distribution(const param_type& params_) noexcept
			: params(params_) {}

		void reset() noexcept {
			val_avail = false;
		}

		template <typename UniformRandomBitGenerator>
		result_type operator()(UniformRandomBitGenerator& g) noexcept(noexcept(g.operator()())) {
			return this->operator()(g, params);
		}

		// Marsaglia's polar method
		// Luc Devroye, Non-Uniform Random Variate Generation, Ch. 5 Uniform and Exponential Spacings, Section 4: The polar method
		// avail for free @ http://luc.devroye.org/rnbookindex.html
		template <typename UniformRandomBitGenerator>
		result_type operator()(UniformRandomBitGenerator& g, const param_type& p) noexcept(noexcept(g.operator()())) {
			if (val_avail) {
				val_avail = false;
				return p.mean() + val * p.stddev();
			}

			// generates a random number \in [-1 ; 1]
			auto gen_rand = [&g]() -> result_type {
				return result_type(g() - g.min()) / result_type(g.max() - g.min()) * result_type(2.0) - result_type(1.0);
			};
			result_type x, y, z;
			do {
				x = gen_rand();
				y = gen_rand();
				z = x*x + y*y;
			} while (z <= result_type(0.) || z >= result_type(1.));

			const auto coeff = std::sqrt(-2 * std::log(z) / z);

			val_avail = true;
			val = y * coeff;

			return p.mean() + x * coeff * p.stddev();
		}

		constexpr param_type param() const noexcept {
			return params;
		}

		void param(const param_type& p) noexcept {
			params = p;
		}

		constexpr result_type mean() const noexcept {
			return params.mean();
		}

		constexpr result_type stddev() const noexcept {
			return params.stddev();
		}

		constexpr result_type min() const noexcept {
			return std::numeric_limits<result_type>::lowest();
		}

		constexpr result_type max() const noexcept {
			return std::numeric_limits<result_type>::max();
		}

		constexpr friend bool operator==(const normal_distribution& lhs, const normal_distribution& rhs) noexcept {
			return lhs.params == rhs.params && lhs.val_avail == rhs.val_avail
			                 && (!lhs.val_avail || lhs.val == rhs.val);
		}

		constexpr friend bool operator!=(const normal_distribution& lhs, const normal_distribution& rhs) noexcept {
			return !(lhs == rhs);
		}

		template <typename CharT, typename Traits>
		friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const normal_distribution& d) noexcept(noexcept(os.operator<<(std::declval<result_type>()))) {
			os << d.mean() << ' ' << d.stddev() << ' ' << d.val_avail;
			if (d.val_avail) {
				os << d.val;
			}
			return os;
		}

		template <typename CharT, typename Traits>
		friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, normal_distribution& d) noexcept(noexcept(is.operator>>(std::declval<result_type>()))) {
			result_type mean{}, stddev{};
			is >> mean >> stddev >> d.val_avail;
			if (d.val_avail) {
				is >> d.val;
			}
			d.param({mean, stddev});
			return is;
		}

	private:
		result_type val{};
		bool val_avail{false};
		param_type params;
	};


	// std::uniform_int_distribution is not portable across compilers
	template <typename IntType = int>
	class uniform_int_distribution {
		static_assert(std::is_integral_v<IntType>, "template argument must be an integral type");
	public:
		using result_type = IntType;
		struct param_type {
			using distribution_type = uniform_int_distribution<IntType>;

			explicit constexpr param_type(IntType a_ = 0, IntType b_ = std::numeric_limits<IntType>::max()) noexcept : m_a(a_), m_b(b_) {}

			constexpr IntType a() const noexcept {
				return m_a;
			}

			constexpr IntType b() const noexcept {
				return m_b;
			}

			constexpr bool operator==(const param_type& other) const noexcept {
				return m_a == other.m_a && m_b == other.m_b;
			}

			constexpr bool operator!=(const param_type& other) const {
				return !(*this == other);
			}


		private:
			IntType m_a, m_b;
		};

		constexpr uniform_int_distribution() noexcept
			: uniform_int_distribution(0) {}
		explicit constexpr uniform_int_distribution(IntType a, IntType b = std::numeric_limits<IntType>::max()) noexcept
			: uniform_int_distribution(param_type(a, b)) {}
		explicit constexpr uniform_int_distribution(const param_type& p) noexcept
			: params(p) {}

		constexpr void reset() const noexcept {}

		template <typename UniformRandomBitGenerator>
		result_type operator()(UniformRandomBitGenerator& g) const noexcept(noexcept(g.operator()())) {
			return this->operator()(g, params);
		}

		template <typename UniformRandomBitGenerator>
		result_type operator()(UniformRandomBitGenerator& g, const param_type& p) const noexcept(noexcept(g.operator()())) {
			using unsigned_rng_res_t = std::make_unsigned_t<typename UniformRandomBitGenerator::result_type>;
			using unsigned_dist_res_t = std::make_unsigned_t<result_type>;

			unsigned_rng_res_t rng_range = g.max() - g.min(); // not adding 1 because of potential overflows
			unsigned_dist_res_t dist_range = p.b() - p.a();

			if (rng_range != std::numeric_limits<unsigned_rng_res_t>::max()) {
				++rng_range;
			}
			if (dist_range != std::numeric_limits<unsigned_dist_res_t>::max()) {
				++dist_range;
			}

			using unsigned_comm_t = std::common_type_t<unsigned_rng_res_t, unsigned_dist_res_t>;

			if (rng_range > dist_range) {
				// downscaling

				const unsigned_comm_t int_scale = rng_range / dist_range;
				const unsigned_comm_t gen_max = int_scale * rng_range;

				unsigned_comm_t value;
				while ((value = g() - g.min()) > gen_max);
				return result_type(value / int_scale) + p.a();

			} else if (rng_range < dist_range) {
				// upscaling

				unsigned_comm_t int_scale = dist_range / rng_range;

				unsigned_comm_t value;
				unsigned_comm_t tmp; // overflow control, credit to GNU ISO C++ library [didn't think about it myself]
				// technically overflow is UB but I doubt it can be optimized away by compilers
				do {
					tmp = rng_range * this->operator()(g, param_type{result_type(0), result_type(int_scale)});
					value = tmp + (g() - g.min());
				} while (value > dist_range || value < tmp);

				return result_type(value - g.min()) + p.a();

			} else {
				return result_type(g() - g.min()) + p.a();
			}
		}

		constexpr param_type param() const noexcept {
			return params;
		}

		void param(const param_type& p) noexcept {
			params = p;
		}

		constexpr result_type a() const noexcept {
			return params.a();
		}

		constexpr result_type b() const noexcept {
			return params.b();
		}

		constexpr result_type min() const noexcept {
			return a();
		}

		constexpr result_type max() const noexcept {
			return b();
		}

		constexpr friend bool operator==(const uniform_int_distribution& lhs, const uniform_int_distribution& rhs) noexcept {
			return lhs.params == rhs.params;
		}

		constexpr friend bool operator!=(const uniform_int_distribution& lhs, const uniform_int_distribution& rhs) noexcept {
			return !(lhs == rhs);
		}

		template <typename CharT, typename Traits>
		friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const uniform_int_distribution& d) noexcept(noexcept(os.operator<<(std::declval<result_type>()))) {
			os << d.a() << ' ' << d.b();
			return os;
		}

		template <typename CharT, typename Traits>
		friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, uniform_int_distribution& d) noexcept(noexcept(is.operator>>(std::declval<result_type>()))) {
			result_type a{}, b{};
			is >> a >> b;
			d.param({a, b});
			return is;
		}

	private:
		param_type params;
	};
}

#endif //DUNGEEP_RANDOM_HPP
