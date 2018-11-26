//
//  blocksci_iterator_type.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/25/18.
//

#ifndef blocksci_iterator_type_hpp
#define blocksci_iterator_type_hpp

#include "python_fwd.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <mpark/variant.hpp>

#include <pybind11/pybind11.h>

#include <any>
#include <chrono>

using BlocksciIteratorTypeVariant = mpark::variant<
	RawIterator<blocksci::Block>,
	RawIterator<blocksci::Transaction>,
	RawIterator<blocksci::Input>,
	RawIterator<blocksci::Output>,
	RawIterator<blocksci::AnyScript>,
	RawIterator<blocksci::EquivAddress>,
	RawIterator<blocksci::Cluster>,
	RawIterator<blocksci::TaggedCluster>,
	RawIterator<blocksci::TaggedAddress>,
	RawIterator<blocksci::script::Pubkey>,
	RawIterator<blocksci::script::PubkeyHash>,
	RawIterator<blocksci::script::WitnessPubkeyHash>,
	RawIterator<blocksci::script::MultisigPubkey>,
	RawIterator<blocksci::script::Multisig>,
	RawIterator<blocksci::script::ScriptHash>,
	RawIterator<blocksci::script::WitnessScriptHash>,
	RawIterator<blocksci::script::OpReturn>,
	RawIterator<blocksci::script::Nonstandard>,
	RawIterator<blocksci::script::WitnessUnknown>,
	RawIterator<blocksci::AddressType::Enum>,
	RawIterator<int64_t>,
	RawIterator<bool>,
	RawIterator<std::chrono::system_clock::time_point>,
	RawIterator<blocksci::uint256>,
	RawIterator<blocksci::uint160>,
	RawIterator<pybind11::bytes>,
	RawIterator<pybind11::list>,
	RawIterator<std::string>
>;

struct BlocksciType;

struct BlocksciIteratorType {
	BlocksciIteratorTypeVariant var;

	template <typename U, std::enable_if_t<std::is_convertible<U, BlocksciIteratorTypeVariant>::value> * = nullptr>
	explicit BlocksciIteratorType(U && t) : var(std::forward<U>(t)) {}

	BlocksciIteratorType(const BlocksciIteratorType &val) = default;
	BlocksciIteratorType(BlocksciIteratorType && val) = default;

	std::any toAny() const;
	pybind11::object toObject() const;
	RawIterator<BlocksciType> toGeneric();
	RawIterator<std::any> toAnySequence();
};



#endif /* blocksci_iterator_type_hpp */
