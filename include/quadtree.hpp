#ifndef DUNGEEP_QUADTREE_HPP
#define DUNGEEP_QUADTREE_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018, Lucas Lazare                                                                                                     ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  		files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,  ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  		is furnished to do so, subject to the following conditions:                                                                 ///
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
#include <iterator>
#include <utility>
#include <memory>

#include "geometry.hpp"

namespace dungeep {

	template <typename PointerType>
	struct quadtree_wrapper {

		quadtree_wrapper(PointerType ptr) : value(std::move(ptr)) {}

		quadtree_wrapper(const quadtree_wrapper&) = delete;
		quadtree_wrapper(quadtree_wrapper&&) noexcept = default;

		quadtree_wrapper& operator=(const quadtree_wrapper&) = delete;
		quadtree_wrapper& operator=(quadtree_wrapper&&) noexcept = default;

		PointerType& get() noexcept {
			return value;
		}

		decltype(auto) operator->() noexcept(noexcept(std::declval<PointerType>().operator->())) {
			return value.operator->();
		}

		decltype(auto) operator*() noexcept(noexcept(std::declval<PointerType>().operator*())) {
			return *value;
		}

		area<float> hitbox() const noexcept {
			return value->hitbox();
		}

		void set_hitbox(area<float> ar) noexcept {
			value->set_hitbox(ar);
		}

		PointerType value;
	};

	template <typename T>
	struct quadtree_wrapper<T*> {
		T* get() noexcept {
			return value;
		}

		const T* get() const noexcept {
			return value;
		}

		T& operator->() noexcept {
			return *value;
		}

		const T& operator->() const noexcept {
			return *value;
		}

		T& operator*() noexcept {
			return *value;
		}

		const T& operator*() const noexcept {
			return *value;
		}

		area<float> hitbox() const noexcept {
			return value->hitbox();
		}

		void set_hitbox(area<float> ar) noexcept {
			value->set_hitbox(ar);
		}

		T* value;
	};

	template <typename T>
	using qtree_unique_ptr = quadtree_wrapper<std::unique_ptr<T>>;

	template <typename T>
	using qtree_shared_ptr = quadtree_wrapper<std::shared_ptr<T>>;

	enum class quadtree_dynamics {
		static_children,   // children are created at the start
		lazy_children,     // children are created when needed
		dynamic_children,  // children are created when needed, deleted when unneeded
	};

	// T should have a noexcept '.hitbox()' method returning an area<float>.
	// T should have a '.set_hitbox(area<float>)' method.
	template <typename T, quadtree_dynamics Dynamicity = quadtree_dynamics::dynamic_children, template <typename...> typename Container = std::vector>
	class quadtree {
		template <typename, typename, typename>
		struct iterator_type;

	public:

		using container = Container<T>;
		using value_type = typename container::value_type;
		using allocator_type = typename container::allocator_type;
		using size_type = typename container::size_type;
		using difference_type = typename container::difference_type;
		using reference = typename container::reference;
		using const_reference = typename container::const_reference;
		using pointer = typename container::pointer;
		using const_pointer = typename container::const_pointer;
		using iterator = iterator_type<quadtree<T,Dynamicity,Container>, value_type, typename container::iterator>;
		using const_iterator = iterator_type<const quadtree<T,Dynamicity,Container>, const value_type, typename container::const_iterator>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using area = dungeep::area<float>;
		using point = dungeep::point<float>;

	public:
		explicit quadtree(const area& ar) // NOLINT
			noexcept(Dynamicity != quadtree_dynamics::static_children && noexcept(container()))
			: quadtree(ar, 10, 20) {}

		quadtree(const area&, size_type max_depth, size_type max_size)
			noexcept(Dynamicity != quadtree_dynamics::static_children && noexcept(container()));

		quadtree(const quadtree& other);
		quadtree(quadtree&& other) noexcept(noexcept(container(std::declval<container&&>()))) = default;

		quadtree<T,Dynamicity,Container>& operator=(const quadtree<T,Dynamicity,Container>& other);
		quadtree<T,Dynamicity,Container>& operator=(quadtree<T,Dynamicity,Container>&& other) noexcept(noexcept(container().operator=(std::declval<container&&>()))) = default;

		// Iterator for the whole collection
		[[nodiscard]] iterator begin() noexcept;
		[[nodiscard]] const_iterator begin() const noexcept;
		[[nodiscard]] const_iterator cbegin() const noexcept;

		[[nodiscard]] iterator end() noexcept;
		[[nodiscard]] const_iterator end() const noexcept;
		[[nodiscard]] const_iterator cend() const noexcept;


		/**
		 * Inserts an element, given its '.hitbox()' location
		 * Iterators are invalidated
		 */
		iterator insert(const value_type& value);

		/**
		 * Iterators are invalidated
		 */
		template <typename... Args>
		iterator emplace(const area& target, Args&&... args);

		/**
		 * Visists all elements on the given area
		 * The visitor function should not attempt to insert or remove an element in or from the collection.
		 * If the visitor returns true, the element is safely deleted (iterators are invalidated).
		 *
		 * Do not attempt to call quatree::erase() while visiting. To delete current value, return true instead.
		 */
		template <typename FuncT>
		std::enable_if_t<std::is_invocable_v<FuncT, iterator>>
		visit(const area& target, FuncT&& visitor) noexcept(std::is_nothrow_invocable_v<FuncT, iterator>);

		template <typename FuncT>
		std::enable_if_t<std::is_invocable_v<FuncT, const_iterator>>
		visit(const area& target, FuncT&& visitor) const noexcept(std::is_nothrow_invocable_v<FuncT, const_iterator>);


		[[nodiscard]] bool empty() const noexcept;

		[[nodiscard]] size_type size() const noexcept;

		void clear() noexcept(noexcept(container().clear()));

		/**
		 * returns the element right after the erased one
		 * Iterators are invalidated
		 */
		iterator erase(iterator it);
		iterator erase(const_iterator it);
		void erase(const T&);

		/**
		 * Returns true if at least one element is at least partially present in the given area
		 */
		bool has_collision(const area& ar) const noexcept;

		/**
		 * Same as without 'pred', but the colliding element must be an argument for which pred returned true
		 * 'pred' should take 'T&'/'const T&' as single parameter.
		 * Elements passed to the predicate all collides
		 */
		template <typename FuncT>
		bool has_collision_if(const area& ar, FuncT&& pred) noexcept(std::is_nothrow_invocable_v<FuncT, T&>);
		template <typename FuncT>
		bool has_collision_if(const area& ar, FuncT&& pred) const noexcept(std::is_nothrow_invocable_v<FuncT, const T&>);

		/**
		 * Iterators are invalidated
		 */
		T extract(iterator element);
		T extract(const_iterator element);

		iterator find(const T& element) noexcept;
		const_iterator find(const T& element) const noexcept;

		/**
		 * Iterators are invalidated
		 */
		void move(iterator it, const area& new_area);
		void move(const T& element, const area& new_area);
		void move(const_iterator it, const area& new_area);


	private:


		struct children {
			children(const area& shared_area, size_type max_depth, size_type max_size);
			children(const children& other) = default;

			static area split_from_indexed_dir(const area& shared, int dir);

			auto& operator[](std::size_t i) { return children_[i]; }
			const auto& operator[](std::size_t i) const { return children_[i]; }
		private:
			std::array<quadtree<T,Dynamicity,Container>,4> children_;
		};

		struct dirs_struct {
			enum dirs_enum : unsigned {
				top_left,
				top_right,
				bot_right,
				bot_left,
				none
			};
		};
		using dirs = typename dirs_struct::dirs_enum;

		void create_children();

		[[nodiscard]] dirs find_dir(const area&) const;

		// preserves iterator
		template <typename IteratorType>
		void delete_children(IteratorType&);

		template <typename IteratorType>
		IteratorType erase_impl(IteratorType);

		template <typename IteratorType>
		T extract_impl(IteratorType element);

		template <typename Iterator>
		void move_impl(Iterator it, const area& new_area);

		area area_;
		point center_;
		size_type max_size_;
		size_type max_depth_;

		container values_;

		std::unique_ptr<children> children_;

		friend const_iterator;
		friend iterator;
	};
}


#include "quadtree.tpp"


#endif //DUNGEEP_QUADTREE_HPP
