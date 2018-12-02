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
///  		warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or       ///
///  copyright holders X be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,      ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <iterator>
#include <utility>
#include <memory>

#include "geometry.hpp"

namespace dungeep {

	enum class quadtree_dynamics {
		static_children,   // children are created at the start
		lazy_children,     // children are created when needed
		dynamic_children,  // children are created when needed, deleted when uneeded
	};

	// T should have a noexcept '.hitbox()' method returning an area.
	template <typename T, quadtree_dynamics Dynamicity = quadtree_dynamics::dynamic_children, template <typename> typename Container = std::vector>
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

	public:
		explicit quadtree(const area& ar)
			noexcept(Dynamicity != quadtree_dynamics::static_children && noexcept(container()))
			: quadtree(ar, 10, 50) {}

		quadtree(const area&, size_type max_depth, size_type max_size)
			noexcept(Dynamicity != quadtree_dynamics::static_children && noexcept(container()));

		quadtree(const quadtree& other);
		quadtree(quadtree&& other) noexcept(noexcept(container(std::declval<container&&>()))) = default;

		quadtree<T,Dynamicity,Container>& operator=(const quadtree<T,Dynamicity,Container>& other);
		quadtree<T,Dynamicity,Container>& operator=(quadtree<T,Dynamicity,Container>&& other) noexcept(noexcept(container().operator=(std::declval<container&&>()))) = default;

		[[nodiscard]] iterator begin() noexcept;
		[[nodiscard]] const_iterator begin() const noexcept;
		[[nodiscard]] const_iterator cbegin() const noexcept;

		[[nodiscard]] iterator end() noexcept;
		[[nodiscard]] const_iterator end() const noexcept;
		[[nodiscard]] const_iterator cend() const noexcept;

		iterator insert(const value_type& value);

		template <typename... Args>
		iterator emplace(const area& target, Args&&... args);

		/**
		 * Visists all elements on the given area
		 * TODO: visit([const] T&)
		 */
		template <typename FuncT>
		void visit(const area& target, FuncT&& visitor) noexcept(std::is_nothrow_invocable_v<FuncT, iterator>);
		template <typename FuncT>
		void visit(const area& target, FuncT&& visitor) const noexcept(std::is_nothrow_invocable_v<FuncT, const_iterator>);


		[[nodiscard]] bool empty() const noexcept;

		[[nodiscard]] size_type size() const noexcept;

		void clear() noexcept(noexcept(container().clear()));

		/**
		 * returns the element right after the erased one
		 * TODO: iterator invalidation ?
		 * TODO: erase(const T&)
		 */
		iterator erase(iterator it);
		iterator erase(const_iterator it);

		/**
		 * Moves an element to another area
		 * TODO: return iterator ? iterator invalidation ?
		 * TODO: update_pos(const T&)
		 */
		void update_pos(iterator it);
		void update_pos(const_iterator it);

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
		 * TODO: iterator invalidation?
		 */
		T extract(iterator element);
		T extract(const_iterator element);

		iterator find(const T& element) noexcept;
		const_iterator find(const T& element) const noexcept;


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
			enum dirs_enum {
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
		void update_pos_impl(IteratorType it);

		template <typename IteratorType>
		T extract_impl(IteratorType element);

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
