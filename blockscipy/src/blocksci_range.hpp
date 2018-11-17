//
//  blocksci_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_blocksci_range_h
#define blocksci_blocksci_range_h

#include "python_fwd.hpp"
#include "generic_sequence.hpp"

#include <blocksci/scripts/script_variant.hpp>

#include <pybind11/pybind11.h>

template <typename T>
struct RangeClasses {
	pybind11::class_<Sequence<T>> sequence;
    pybind11::class_<Iterator<T>, Sequence<T>, GenericIterator> iterator;
    pybind11::class_<Range<T>, Sequence<T>, GenericRange> range;
};

template <blocksci::AddressType::Enum type>
struct RangeClasses<blocksci::ScriptAddress<type>> {
	pybind11::class_<Sequence<blocksci::ScriptAddress<type>>> sequence;
    pybind11::class_<Iterator<blocksci::ScriptAddress<type>>, Sequence<blocksci::ScriptAddress<type>>, GenericAddressIterator> iterator;
    pybind11::class_<Range<blocksci::ScriptAddress<type>>, Sequence<blocksci::ScriptAddress<type>>, GenericAddressRange> range;
};

template <>
struct RangeClasses<blocksci::AnyScript> {
	pybind11::class_<Sequence<blocksci::AnyScript>> sequence;
    pybind11::class_<Iterator<blocksci::AnyScript>, Sequence<blocksci::AnyScript>, GenericAddressIterator> iterator;
    pybind11::class_<Range<blocksci::AnyScript>, Sequence<blocksci::AnyScript>, GenericAddressRange> range;
};

#endif /* blocksci_blocksci_range_h */
