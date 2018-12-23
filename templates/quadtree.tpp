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


#include <cassert>
#include <iterator>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace dungeep {

template <typename T,quadtree_dynamics D, template <typename...> typename U>
template <typename QuadTree, typename Value, typename SubIterator>
struct quadtree<T,D,U>::iterator_type {

	using difference_type = std::size_t;
	using value_type = Value;
	using pointer = Value*;
	using reference = Value&;
	using iterator_category = std::bidirectional_iterator_tag;

	iterator_type() noexcept : qt_{nullptr}, current_{}, child_it_{nullptr} {}

	iterator_type(QuadTree& qt) noexcept : qt_{&qt}, current_{qt_->values_.begin()}, child_it_{init_child()} {}

	iterator_type(const iterator_type& other);
	iterator_type& operator=(const iterator_type& other);

	iterator_type(iterator_type&& other) noexcept = default;

	iterator_type operator++(int) const noexcept;
	iterator_type& operator++() noexcept;
	iterator_type operator--(int) const noexcept;
	iterator_type& operator--() noexcept;

	bool operator==(const iterator_type& other) const noexcept;
	bool operator!=(const iterator_type& other) const noexcept;

	bool operator>(const iterator_type& other) const noexcept;
	bool operator>=(const iterator_type& other) const noexcept;
	bool operator<(const iterator_type& other) const noexcept;
	bool operator<=(const iterator_type& other) const noexcept;

	Value& operator*() noexcept;
	const Value& operator*() const noexcept;

	Value* operator->() noexcept;
	const Value* operator->() const noexcept;


	bool is_at_beg() const noexcept {
		return current_ == qt_->values_.begin();
	}

	bool is_at_end() const noexcept {
		if (qt_ == nullptr) {
			return true;
		}
		if (current_ != qt_->values_.end()) {
			return false;
		}
		if (!child_it_) {
			return true;
		}
		if (!child_it_->second.is_at_end()) {
			return false;
		}

		for (auto i = child_it_->first ; i < 4 ; ++i) {
			if (!(*qt_->children_)[i].empty()) {
				return false;
			}
		}
		return true;
	}

private:
	friend quadtree<T,D,U>;

	void seek_last() noexcept {
		current_ = std::next(qt_->values_.begin(), static_cast<long>(qt_->values_.size() - 1));
	}

	using pair_type = std::pair<unsigned, iterator_type<QuadTree, Value, SubIterator>>;
	std::unique_ptr<pair_type> init_child() {
		if (qt_->children_) {
			for (auto i = 0u ; i < 4 ; ++i) {
				if (!(*qt_->children_)[i].empty()) {
					return std::make_unique<pair_type>(i + 1, (*qt_->children_)[i].begin());
				}
			}
			return std::make_unique<pair_type>(4, (*qt_->children_)[3].end());
		} else {
			return {nullptr};
		}
	}

	QuadTree* qt_;
	SubIterator current_;
	std::unique_ptr<pair_type> child_it_;

};

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
quadtree<T,D,U>::iterator_type<Q,V,S>::iterator_type(const iterator_type<Q,V,S>& other)
		: qt_{other.qt_}
		, current_{other.current_}
		, child_it_{other.child_it_ == nullptr ? nullptr : std::make_unique<pair_type>(*other.child_it_)}
{}

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
auto quadtree<T,D,U>::iterator_type<Q,V,S>::operator=(const iterator_type& other) -> iterator_type& {
	this->qt_ = other.qt_;
	this->current_ = other.current_;

	if (other.child_it_) {
		child_it_ = std::make_unique<pair_type>(*other.child_it_);
	} else {
		child_it_.reset();
	}
	return *this;
}



template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
auto quadtree<T,D,U>::iterator_type<Q,V,S>::operator++(int) const noexcept -> iterator_type {
        iterator_type<Q,V,S> tmp(*this);
		return ++tmp;
}


template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
auto quadtree<T,D,U>::iterator_type<Q,V,S>::operator++() noexcept -> iterator_type& {
	assert(qt_);
	if (current_ == qt_->values_.end()) {
		assert(child_it_);
		assert(qt_->size() > qt_->values_.size());

		if (!child_it_->second.is_at_end()) {
			++child_it_->second;
		}
		while (child_it_->second.is_at_end() && child_it_->first < 4) {
			child_it_->second = {(*qt_->children_)[child_it_->first]};
			++child_it_->first;
		}
		return *this;
	} else {
		++current_;
		return *this;
	}
}


template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
auto quadtree<T,D,U>::iterator_type<Q,V,S>::operator--(int) const noexcept -> iterator_type {
	iterator_type<Q,V,S> tmp(*this);
	return --tmp;
}

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
auto quadtree<T,D,U>::iterator_type<Q,V,S>::operator--() noexcept -> iterator_type& {
	assert(qt_);
	if (current_ == qt_->values_.end() && child_it_) {
		if (!child_it_->second.is_at_beg()) {
			--child_it_->second;
			return *this;
		}

		while (child_it_->first > 0 && (*(qt_->children_))[child_it_->first - 1].empty()) {
			--child_it_->first;
		}

		child_it_->second = {(*qt_->children_)[child_it_->first]};
		if (child_it_->first == 0) {
			--current_;
		} else {
			child_it_->second.seek_last();
		}
		return *this;
	}
	assert(current_ != qt_->values_.begin());
	--current_;
	return *this;
}

// test end == end, beg == beg, end == {}
template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
bool quadtree<T,D,U>::iterator_type<Q,V,S>::operator==(const iterator_type& other) const noexcept {
	if (other.qt_ == nullptr) {
		return this->is_at_end();
	} else if (this->qt_ == nullptr) {
		return other.is_at_end();
	}

	if (this->qt_ == other.qt_ && this->current_ == other.current_) {
		if (!this->child_it_) {
			return !other.child_it_;
		}
		return this->child_it_->first == other.child_it_->first && this->child_it_->second == other.child_it_->second;
	} else {
		return false;
	}
}

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
bool quadtree<T,D,U>::iterator_type<Q,V,S>::operator!=(const iterator_type& other) const noexcept {
	return !(*this == other);
}

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
bool quadtree<T,D,U>::iterator_type<Q,V,S>::operator>(const iterator_type& other) const noexcept {
	if (this->current_ != other.current_) {
		return this->current_ > other.current_;
	}
	if (this->child_it_) {
		if (other.child_it_) {
			if (this->child_it_.first != other.child_it_.first) {
				return this->child_it_.first > other.child_it_.first;
			} else {
				return this->child_it_.second > other.child_it_.second;
			}
		} else {
			return true;
		}
	} else {
		return false;
	}
}

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
bool quadtree<T,D,U>::iterator_type<Q,V,S>::operator>=(const iterator_type& other) const noexcept {
	if (this->current_ != other.current_) {
		return this->current_ >= other.current_;
	}
	if (this->child_it_) {
		if (other.child_it_) {
			if (this->child_it_.first != other.child_it_.first) {
				return this->child_it_.first > other.child_it_.first;
			} else {
				return this->child_it_.second >= other.child_it_.second;
			}
		} else {
			return true;
		}
	} else {
		return other.child_it_;
	}
}

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
bool quadtree<T,D,U>::iterator_type<Q,V,S>::operator<(const iterator_type& other) const noexcept {
	return !(*this >= other);
}

template <typename T, quadtree_dynamics D,template <typename...> typename U>
template <typename Q, typename V, typename S>
bool quadtree<T,D,U>::iterator_type<Q,V,S>::operator<=(const iterator_type& other) const noexcept {
	return !(*this > other);
}

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
auto quadtree<T,D,U>::iterator_type<Q,V,S>::operator*() noexcept -> value_type& {
	assert(qt_);
	if (current_ != qt_->values_.end()) {
		return *current_;
	} else {
		assert(child_it_);
		return *child_it_->second;
	}
}

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
auto quadtree<T,D,U>::iterator_type<Q,V,S>::operator*() const noexcept -> const value_type& {
	assert(qt_);
	if (current_ != qt_->values_.end()) {
		return *current_;
	} else {
		return *child_it_->second;
	}
}

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
auto quadtree<T,D,U>::iterator_type<Q,V,S>::operator->() noexcept -> value_type* {
	assert(qt_);
	if (current_ != qt_->values_.end()) {
		return &*current_;
	} else {
		return &(*child_it_->second);
	}
}

template <typename T, quadtree_dynamics D, template <typename...> typename U>
template <typename Q, typename V, typename S>
auto quadtree<T,D,U>::iterator_type<Q,V,S>::operator->() const noexcept -> const value_type* {
	if (current_ != qt_->values_.end()) {
		return &*current_;
	} else {
		return &*child_it_->second;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T,quadtree_dynamics Dynamicity, template <typename...> typename Container>
quadtree<T, Dynamicity, Container>::quadtree(const area& ar, size_type max_depth, size_type max_size)
	noexcept(Dynamicity != quadtree_dynamics::static_children && noexcept(container()))
	: area_{ar}
	, center_{(area_.top_left + area_.bot_right) / 2}
	, max_size_{max_size}
	, max_depth_{max_depth}
	, values_{}
	, children_{nullptr}
{
	if constexpr (Dynamicity == quadtree_dynamics::static_children) {
		if (max_depth_ > 0) {
			children_ = std::make_unique<children>(area_, max_depth_ - 1, max_size_);
		}
	}
	values_.reserve(max_size);
}


template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
quadtree<T, Dynamicity, Container>::quadtree(const quadtree& other)
	: area_{other.area_}
	, center_{other.center_}
	, max_size_{other.max_size_}
	, max_depth_{other.max_depth_}
	, values_{other.values_}
	, children_{std::make_unique<children>(other.children_)}
{}

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
auto quadtree<T, Dynamicity, Container>::operator=(const quadtree<T, Dynamicity, Container>& other) -> quadtree& {
	this->area_ = other.area_;
	this->center_ = other.center_;
	this->max_size_ = other.max_size_;
	this->max_depth_ = other.max_depth_;
	this->values_ = other.values_;
	this->children_ = std::make_unique<children>(other.children_);
	return *this;
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::begin() noexcept -> iterator {
	return iterator{*this};
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::begin() const noexcept -> const_iterator {
	return iterator{*this};
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::cbegin() const noexcept -> const_iterator {
	return const_iterator{*this};
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::end() noexcept -> iterator {
	return iterator{};
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::end() const noexcept -> const_iterator {
	return const_iterator{};
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::cend() const noexcept -> const_iterator {
	return const_iterator{};
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::insert(const value_type& value) -> iterator {
	return emplace(value.hitbox(), value);
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
template<typename... Args>
auto quadtree<T, D, Container>::emplace(const area& target, Args&& ... args) -> iterator {

	if (values_.size() == max_size_) {
		create_children();
	}

	dirs target_pos = dirs::none;
	if (children_) {
		target_pos = find_dir(target);
	}

	iterator it{*this};
	if (target_pos == dirs::none) {
		values_.emplace_back(std::forward<Args>(args)...);
		it.current_ = std::prev(values_.end());
	} else {
		it.current_ = values_.end();
		it.child_it_->first = target_pos + 1;
		it.child_it_->second = (*children_)[target_pos].emplace(target, args...);
	}
	return it;
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
bool quadtree<T, D, Container>::empty() const noexcept {
	if (!values_.empty()) {
		return false;
	}
	if (!children_) {
		return true;
	}

	return (*children_)[0].empty() && (*children_)[1].empty() && (*children_)[2].empty() && (*children_)[3].empty();
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::size() const noexcept -> size_type {
	if (!children_) {
		return values_.size();
	}
	size_type size = values_.size();
	for (auto i = 0u ; i < 4 ; ++i) {
		size += (*children_)[i].size();
	}
	return size;
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
void quadtree<T, D, Container>::clear() noexcept(noexcept(container().clear())) {
	values_.clear();
	if (children_) {
		for (auto i = 0u ; i < 4 ; ++i) {
			(*children_)[i].clear();
		}
	}
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::erase(iterator it) -> iterator {
	return erase_impl(it);
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::erase(const_iterator it) -> iterator {
	return erase_impl(it);
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
void quadtree<T, D, Container>::erase(const T& t) {
	auto dir = find_dir(t.hitbox());

	if (children_ && dir != dirs::none) {
		(*children_)[dir].erase(t);
		iterator null_it{};
		delete_children(null_it);
		return;
	}

	auto it = std::find_if(values_.begin(), values_.end(), [&t](const T& t2) { return t == t2; });
	if (it != values_.end()) {
		if (it + 1 != values_.end()) {
			*it = std::move_if_noexcept(values_.back());
		}
		values_.pop_back();
	}
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
bool quadtree<T, D, Container>::has_collision(const area& ar) const noexcept {
	return has_collision_if(ar, [](auto&&) { return true; });
}

#define DUNGEEP_QTREE_HASCOLLISIONIF_IMPL(ar, pred) \
	if (!ar.collides_with(this->area_)) {\
		return false;\
	}\
	\
	for (auto&& val : values_) {\
		if (ar.collides_with(val.hitbox()) && pred(val)) {\
			return true;\
		}\
	}\
	\
	if (children_) {\
		for (auto i = 0u ; i < 4 ; ++i) {\
			if ((*children_)[i].has_collision_if(ar, pred)) {\
				return true;\
			}\
		}\
	}\
	\
	return false;

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
template <typename FuncT>
bool quadtree<T,D,Container>::has_collision_if(const area& ar, FuncT&& pred) noexcept(std::is_nothrow_invocable_v<FuncT, T&>) {
	DUNGEEP_QTREE_HASCOLLISIONIF_IMPL(ar, pred)
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
template <typename FuncT>
bool quadtree<T,D,Container>::has_collision_if(const area& ar, FuncT&& pred) const noexcept(std::is_nothrow_invocable_v<FuncT, const T&>) {
	DUNGEEP_QTREE_HASCOLLISIONIF_IMPL(ar, pred)
}

#undef DUNGEEP_QTREE_HASCOLLISIONIF_IMPL

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
auto quadtree<T, D, Container>::find_dir(const area& target) const -> dirs {

	if (center_.x < target.top_left.x) {
		if (center_.y < target.top_left.y) {
			return dirs::bot_right;
		} else if (target.bot_right.y < center_.y) {
			return dirs::top_right;
		}
	} else if (target.bot_right.x < center_.x){
		if (center_.y < target.top_left.y) {
			return dirs::bot_left;
		} else if (target.bot_right.y < center_.y) {
			return dirs::top_left;
		}
	}
	return dirs::none;
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
template <typename IteratorType>
IteratorType quadtree<T, D, Container>::erase_impl(IteratorType it) {
	assert(it.qt_ == this);

	if (it.current_ != values_.end()) {
		if (it.current_ + 1 != values_.end()) {
			*it.current_ = std::move_if_noexcept(values_.back());
			values_.pop_back();
		} else {
			values_.pop_back();
			it.current_ = values_.end();
		}

		if (it.current_ == values_.end() && it.child_it_) {
			it.child_it_->second = (*children_)[it.child_it_->first].begin();
		}
		return it;
	}
	assert(it != iterator{});
	it.child_it_->second = (*children_)[it.child_it_->first - 1].erase(it.child_it_->second);

	while (it.child_it_->second.is_at_end() && it.child_it_->first < 4) {
		it.child_it_->second = (*it.qt_->children_)[it.child_it_->first].begin();
		++it.child_it_->first;
	}
	delete_children(it);
	return it;
}

#define DUNGEEP_QTREE_VISIT_IMPL(iterator_type, target, visitor) \
	if (!target.collides_with(this->area_)) { \
		return; \
	} \
	\
	{ \
		iterator_type it{*this}; \
		while (it.current_ != this->values_.end()) { \
			if (target.collides_with(it->hitbox())) { \
				/* if non const context AND visitor returns a boolean */\
				if constexpr (std::is_same_v<iterator_type, iterator>) { \
                    if constexpr (std::is_same_v<std::invoke_result_t<FuncT, iterator>, bool>) { \
                        if (visitor(it)) { \
                            if (it.current_ + 1 != this->values_.end()) { \
                                *it.current_ = this->values_.back(); \
                                this->values_.pop_back(); \
                            } else { \
                                this->values_.pop_back(); \
                                it.current_ = values_.end(); /* it is otherwise invalid and thus cannot be safely compared to .end() */ \
                            } \
                            continue; \
                        } \
                    } else { \
                        visitor(it); \
                    } \
				} else { \
					visitor(it); \
				} \
			} \
			++it; \
		} \
	} \
	\
	if (children_) { \
		for (auto i = 0u ; i < 4 ; ++i) { \
			(*children_)[i].visit(target, visitor); \
		} \
	}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
template <typename FuncT>
std::enable_if_t<std::is_invocable_v<FuncT, typename quadtree<T,D,Container>::iterator>>
quadtree<T, D, Container>::visit(const area& target, FuncT&& visitor) noexcept(std::is_nothrow_invocable_v<FuncT, iterator>) {
	DUNGEEP_QTREE_VISIT_IMPL(iterator, target, visitor);
}

template<typename T,quadtree_dynamics D, template <typename...> typename Container>
template <typename FuncT>
std::enable_if_t<std::is_invocable_v<FuncT, typename quadtree<T,D,Container>::const_iterator>>
quadtree<T, D, Container>::visit(const area& target, FuncT&& visitor) const noexcept(std::is_nothrow_invocable_v<FuncT, const_iterator>) {
	DUNGEEP_QTREE_VISIT_IMPL(const_iterator, target, visitor)
}

#undef DUNGEEP_QTREE_VISIT_IMPL

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
void quadtree<T, Dynamicity, Container>::create_children() {
	if constexpr (Dynamicity != quadtree_dynamics::static_children) {
		if (!children_ && max_depth_ > 0) {
			children_ = std::make_unique<children>(area_, max_depth_ - 1, max_size_);

			auto it = values_.begin();
			while (it != values_.end()) {
				dirs dir = find_dir(it->hitbox());
				if (dir != dirs::none) {
					(*children_)[dir].emplace(it->hitbox(), std::move_if_noexcept(*it));
					if (it + 1 != values_.end()) {
						*it = std::move_if_noexcept(values_.back());
						values_.pop_back();
					} else {
						values_.pop_back();
						it = values_.end(); // it is otherwise invalid and thus cannot be safely compared to .end()
					}
				} else {
					++it;
				}
			}
		}
	}
}

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
template<typename IteratorType>
void quadtree<T, Dynamicity, Container>::delete_children(IteratorType& it) {
	if constexpr (Dynamicity == quadtree_dynamics::dynamic_children) {
		if (!children_ || values_.size() > max_size_ / 3) {
			return;
		}

		for (auto i = 0u ; i < 4 ; ++i) {
			if (!(*children_)[i].empty()) {
				return;
			}
		}

		it.child_it_.reset();
		children_.reset();
	}
}

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
T quadtree<T, Dynamicity, Container>::extract(iterator element) {
	return extract_impl(element);
}


template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
T quadtree<T, Dynamicity, Container>::extract(const_iterator element) {
	return extract_impl(element);
}

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
template<typename IteratorType>
T quadtree<T, Dynamicity, Container>::extract_impl(IteratorType element) {
	IteratorType* runner = &element;
	while (runner->current_ == runner->qt_->values_.end()) {
		runner = &runner->child_it_->second;
	}

	T return_value = std::move_if_noexcept(*runner->current_);
	if (runner->current_ + 1 != runner->qt_->values_.end()) {
		*runner->current_ = runner->qt_->values_.back();
	}
	runner->qt_->values_.pop_back();

	IteratorType null_it{};
	delete_children(null_it);

	return return_value;
}

#define DUNGEEP_QTREE_FIND_IMPL(iterator_type) \
	dirs dir = find_dir(element.hitbox()); \
	\
	iterator_type it{*this}; \
	if (dir == dirs::none) { \
		it.current_ = std::find(values_.begin(), values_.end(), element); \
		if (it.current_ == values_.end()) { \
			it = {}; \
		} \
	} else { \
		it.current_ = values_.end(); \
		it.child_it_->first = dir + 1; \
		it.child_it_->second = (*children_)[dir].find(element); \
	} \
	return it; \

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
auto quadtree<T, Dynamicity, Container>::find(const T& element) noexcept -> iterator {
	DUNGEEP_QTREE_FIND_IMPL(iterator)
}

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
auto quadtree<T, Dynamicity, Container>::find(const T& element) const noexcept -> const_iterator {
	DUNGEEP_QTREE_FIND_IMPL(const_iterator)
}

#undef DUNGEEP_QTREE_FIND_IMPL

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
void quadtree<T, Dynamicity, Container>::move(iterator it, const area& new_area) {
	move_impl(it, new_area);
}

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
void quadtree<T, Dynamicity, Container>::move(const T& element, const area& new_area) {
	iterator it = find(element);
	if (it != this->end()) {
		move_impl(it, new_area);
	}
}

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
void quadtree<T, Dynamicity, Container>::move(const_iterator it, const area& new_area) {
	move_impl(it, new_area);
}

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
template<typename Iterator>
void quadtree<T, Dynamicity, Container>::move_impl(Iterator it, const area& new_area) {
	if (!children_) {
		// moving from this->values_ to this->values_
		return;
	}

	dirs new_area_dir = find_dir(new_area);
	dirs current_area_dir = find_dir(it->hitbox());

	if (current_area_dir == new_area_dir) {
		(*children_)[new_area_dir].move(it.child_it_->second, new_area);
		return;
	}

	T val = extract(it);
	val.set_hitbox(new_area);
	emplace(new_area, std::move_if_noexcept(val));
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
quadtree<T, Dynamicity, Container>::children::children(const area& shared_area, size_type max_depth, size_type max_size)
	: children_{{
        {split_from_indexed_dir(shared_area, 0), max_depth, max_size},
        {split_from_indexed_dir(shared_area, 1), max_depth, max_size},
        {split_from_indexed_dir(shared_area, 2), max_depth, max_size},
        {split_from_indexed_dir(shared_area, 3), max_depth, max_size}}}
{}

template<typename T, quadtree_dynamics Dynamicity, template <typename...> typename Container>
auto quadtree<T, Dynamicity, Container>::children::split_from_indexed_dir(const area& shared, int dir) -> area {

	const point center = (shared.top_left + shared.bot_right) / 2;
	const point& top_left = shared.top_left;
	const point& bot_right = shared.bot_right;

	switch (static_cast<dirs>(dir)) {
		case dirs_struct::dirs_enum::top_left:
			return {top_left, center};

		case dirs_struct::dirs_enum::top_right:
			return {point{center.x, top_left.y}, point{bot_right.x, center.y}};

		case dirs_struct::dirs_enum::bot_right:
			return {center, bot_right};

		case dirs_struct::dirs_enum::bot_left:
			return {point{top_left.x, center.y}, point{center.x, bot_right.y}};

		default:
			assert(false);
			return {};
	}
}
}