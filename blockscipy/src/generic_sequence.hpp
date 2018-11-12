//
//  generic_sequence.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef generic_sequence_hpp
#define generic_sequence_hpp

#include "sequence.hpp"
#include "blocksci_type.hpp"

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/transform.hpp>

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

struct GenericSequence {
	virtual RawIterator<BlocksciType> getGenericIterator() = 0;
	virtual ~GenericSequence() = default;
};

template<typename T>
struct Sequence : public GenericSequence {
	virtual RawIterator<T> getIterator() = 0;
	virtual ~Sequence() = default;

	RawIterator<BlocksciType> getGenericIterator() {
		return getIterator() | ranges::view::transform([](T && v) -> BlocksciType {
			return v;
		});
	}
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

	RawIterator<T> getIterator() override {
		return rng;
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

	RawIterator<T> getIterator() override {
		return rng;
	}
};

#endif /* generic_sequence_hpp */
