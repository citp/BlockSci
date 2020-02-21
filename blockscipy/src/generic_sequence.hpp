//
//  generic_sequence.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef generic_sequence_hpp
#define generic_sequence_hpp

#include "python_fwd.hpp"

#include <any>

template <typename T>
struct Iterator;

template <typename T>
struct Range;

template <typename T, ranges::category range_cat>
struct AnyViewType;

struct BlocksciIteratorType;
struct BlocksciRangeType;

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

struct GenericIterator {
	virtual BlocksciIteratorType getGenericIterator() = 0;
	virtual ~GenericIterator() = default;

	RawIterator<std::any> getAllIterator();
};

struct GenericRange : public GenericIterator {
	virtual BlocksciRangeType getGenericRange() = 0;
	virtual int64_t size() = 0;
	virtual bool empty() = 0;
	virtual ~GenericRange() = default;

	BlocksciIteratorType getGenericIterator() override;
	RawRange<std::any> getAllRange();
};

template<typename T>
struct Sequence {
	virtual RawIterator<T> getIterator() = 0;
	virtual ~Sequence() = default;
};

struct GenericAddressIterator: public GenericIterator {

};

struct GenericAddressRange : public GenericRange {

};

#endif /* generic_sequence_hpp */
