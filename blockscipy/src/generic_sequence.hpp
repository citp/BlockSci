//
//  generic_sequence.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef generic_sequence_hpp
#define generic_sequence_hpp

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/transform.hpp>

#include <any>

constexpr ranges::category random_access_sized = ranges::category::random_access | ranges::category::sized;

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

template <ranges::category range_cat>
struct GenericSequence {
	virtual any_view<std::any, range_cat> getAnySequence() = 0;
	virtual ~GenericSequence() = default;
};


template<typename T>
struct Sequence {
	virtual Iterator<T> getIterator() = 0;
	virtual ~Sequence() = default;
};

template <typename T>
struct Iterator : public GenericSequence<ranges::category::input>, public Sequence<T>, public ranges::any_view<T> {
	ranges::any_view<T> rng;

	Iterator(ranges::any_view<T> && r) : rng(std::move(r)) {}

	Iterator(const ranges::any_view<T> &r) : rng(std::move(r)) {}

	Iterator(Range<T> r) : rng(std::move(r.rng)) {}

	auto begin() {
		return rng.begin();
	}

	auto end() {
		return rng.end();
	}

	Iterator<std::any> getAnySequence() override {
		return ranges::any_view<std::any>{ranges::view::transform(rng, [](T && t) -> std::any {
			return std::move(t);
		})};
	}

	Iterator<T> getIterator() override {
		return Iterator<T>{*this};
	}
};

template <typename T>
struct Range : public GenericSequence<random_access_sized>, public Sequence<T> {
	ranges::any_view<T, random_access_sized> rng;

	Range(ranges::any_view<T, random_access_sized> && r) : rng(std::move(r)) {}

	Range(const ranges::any_view<T, random_access_sized> &r) : rng(std::move(r)) {}

	auto begin() {
		return rng.begin();
	}

	auto end() {
		return rng.end();
	}

	Range<std::any> getAnySequence() override {
		return ranges::any_view<std::any, random_access_sized>{ranges::view::transform(rng, [](T && t) -> std::any {
			return std::move(t);
		})};
	}

	Iterator<T> getIterator() override {
		return Iterator<T>{*this};
	}
};

#endif /* generic_sequence_hpp */
