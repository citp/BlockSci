//
//  generic_sequence.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef generic_sequence_hpp
#define generic_sequence_hpp

#include "sequence.hpp"

#include <range/v3/view/any_view.hpp>

template <typename T>
struct Iterator;

template <typename T>
struct Range;

template <typename T, ranges::category range_cat>
struct AnyViewType;

template <typename T>
struct AnyViewType<T, ranges::category::input> {
	using type = Iterator<T>;
};

template <typename T>
struct AnyViewType<T, random_access_sized> {
	using type = Range<T>;
};

template <typename T, ranges::category range_cat>
using any_view = typename AnyViewType<T, range_cat>::type;

template<typename T>
struct Sequence {
	virtual Iterator<T> getIterator() const = 0;
	virtual ~Sequence() = default;
};

template <typename T>
struct Iterator : public Sequence<T> {
	ranges::any_view<T> rng;

	template <typename R>
	Iterator(R && r) : rng(std::forward<R>(r)) {}

	Iterator(Range<T> r) : rng(std::move(r.rng)) {}

	auto begin() {
		return rng.begin();
	}

	auto end() {
		return rng.end();
	}

	Iterator<T> getIterator() const override {
		return *this;
	}
};

template <typename T>
struct Range : public Sequence<T> {
	ranges::any_view<T, random_access_sized> rng;

	Range(ranges::any_view<T, random_access_sized> && r) : rng(std::move(r)) {}

	Range(const ranges::any_view<T, random_access_sized> &r) : rng(r) {}

	auto begin() {
		return rng.begin();
	}

	auto end() {
		return rng.end();
	}

	Iterator<T> getIterator() const override {
		return *this;
	}
};

#endif /* generic_sequence_hpp */
