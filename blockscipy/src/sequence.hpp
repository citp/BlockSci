//
//  sequence.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/4/18.
//

#ifndef blocksci_sequence_hpp
#define blocksci_sequence_hpp

#include "generic_sequence.hpp"
#include "blocksci_iterator_type.hpp"
#include "blocksci_range_type.hpp"

template <typename T>
struct Range : public Sequence<T>, public GenericRange {
	ranges::any_view<T, random_access_sized> rng;

	Range(ranges::any_view<T, random_access_sized> && r) : rng(std::move(r)) {}

	Range(const ranges::any_view<T, random_access_sized> &r) : rng(r) {}

	RawIterator<T> getIterator() override {
		return rng;
	}

	BlocksciRangeType getGenericRange() override {
		return BlocksciRangeType{rng};
	}

	int64_t size() override {
		return rng.size();
	}

	bool empty() override {
		return ranges::empty(rng);
	}
};

template <>
struct Range<blocksci::AnyScript> : public Sequence<blocksci::AnyScript>, public GenericAddressRange {
	using T = blocksci::AnyScript;
	ranges::any_view<T, random_access_sized> rng;

	Range(ranges::any_view<T, random_access_sized> && r) : rng(std::move(r)) {}

	Range(const ranges::any_view<T, random_access_sized> &r) : rng(r) {}

	RawIterator<T> getIterator() override {
		return rng;
	}

	BlocksciRangeType getGenericRange() override {
		return BlocksciRangeType{rng};
	}

	int64_t size() override {
		return rng.size();
	}

	bool empty() override {
		return ranges::empty(rng);
	}
};

template <blocksci::AddressType::Enum type>
struct Range<blocksci::ScriptAddress<type>> : public Sequence<blocksci::ScriptAddress<type>>, public GenericAddressRange {
	using T = blocksci::ScriptAddress<type>;
	ranges::any_view<T, random_access_sized> rng;

	Range(ranges::any_view<T, random_access_sized> && r) : rng(std::move(r)) {}

	Range(const ranges::any_view<T, random_access_sized> &r) : rng(r) {}

	RawIterator<T> getIterator() override {
		return rng;
	}

	BlocksciRangeType getGenericRange() override {
		return BlocksciRangeType{rng};
	}

	int64_t size() override {
		return rng.size();
	}

	bool empty() override {
		return ranges::empty(rng);
	}
};

template <typename T>
struct Iterator : public Sequence<T>, public GenericIterator {
	ranges::any_view<T> rng;

	template <typename R>
	Iterator(R && r) : rng(std::forward<R>(r)) {}

	Iterator(Range<T> r) : rng(std::move(r.rng)) {}

	RawIterator<T> getIterator() override {
		return rng;
	}

	BlocksciIteratorType getGenericIterator() override {
		return BlocksciIteratorType{rng};
	}
};

template <>
struct Iterator<blocksci::AnyScript> : public Sequence<blocksci::AnyScript>, public GenericAddressIterator {
	using T = blocksci::AnyScript;
	ranges::any_view<T> rng;

	template <typename R>
	Iterator(R && r) : rng(std::forward<R>(r)) {}

	Iterator(Range<T> r) : rng(std::move(r.rng)) {}

	RawIterator<T> getIterator() override {
		return rng;
	}

	BlocksciIteratorType getGenericIterator() override {
		return BlocksciIteratorType{rng};
	}
};

template <blocksci::AddressType::Enum type>
struct Iterator<blocksci::ScriptAddress<type>> : public Sequence<blocksci::ScriptAddress<type>>, public GenericAddressIterator {
	using T = blocksci::ScriptAddress<type>;
	ranges::any_view<T> rng;

	template <typename R>
	Iterator(R && r) : rng(std::forward<R>(r)) {}

	Iterator(Range<T> r) : rng(std::move(r.rng)) {}

	RawIterator<T> getIterator() override {
		return rng;
	}

	BlocksciIteratorType getGenericIterator() override {
		return BlocksciIteratorType{rng};
	}
};

#endif /* blocksci_sequence_hpp */
