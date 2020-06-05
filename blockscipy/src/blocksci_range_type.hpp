//
//  blocksci_range_type.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/25/18.
//

#ifndef blocksci_range_type_hpp
#define blocksci_range_type_hpp

#include "python_fwd.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <mpark/variant.hpp>

#include <pybind11/pybind11.h>

#include <any>
#include <chrono>

using BlocksciRangeTypeVariant = mpark::variant<
	RawRange<blocksci::Block>,
	RawRange<blocksci::Transaction>,
	RawRange<blocksci::Input>,
	RawRange<blocksci::Output>,
	RawRange<blocksci::AnyScript>,
	RawRange<blocksci::EquivAddress>,
	RawRange<blocksci::Cluster>,
	RawRange<blocksci::TaggedCluster>,
	RawRange<blocksci::TaggedAddress>,
	RawRange<blocksci::script::Pubkey>,
	RawRange<blocksci::script::PubkeyHash>,
	RawRange<blocksci::script::WitnessPubkeyHash>,
	RawRange<blocksci::script::MultisigPubkey>,
	RawRange<blocksci::script::Multisig>,
	RawRange<blocksci::script::ScriptHash>,
	RawRange<blocksci::script::WitnessScriptHash>,
	RawRange<blocksci::script::OpReturn>,
	RawRange<blocksci::script::Nonstandard>,
	RawRange<blocksci::script::WitnessUnknown>,
	RawRange<blocksci::AddressType::Enum>,
	RawRange<int64_t>,
	RawRange<bool>,
	RawRange<std::chrono::system_clock::time_point>,
	RawRange<blocksci::uint256>,
	RawRange<blocksci::uint160>,
	RawRange<pybind11::bytes>,
	RawRange<pybind11::list>,
	RawRange<std::string>
>;

struct BlocksciIteratorType;
struct BlocksciType;

struct BlocksciRangeType {
	BlocksciRangeTypeVariant var;

	template <typename U, std::enable_if_t<std::is_convertible<U, BlocksciRangeTypeVariant>::value> * = nullptr>
	explicit BlocksciRangeType(U && t) : var(std::forward<U>(t)) {}

	BlocksciRangeType(const BlocksciRangeType &val) = default;
	BlocksciRangeType(BlocksciRangeType && val) = default;

	std::any toAny() const;
	pybind11::object toObject() const;

	BlocksciIteratorType toIterator();
	RawRange<BlocksciType> toGeneric();
	RawRange<std::any> toAnySequence();
};



#endif /* blocksci_range_type_hpp */
