//
//  blocksci_type.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/11/18.
//

#ifndef blocksci_type_hpp
#define blocksci_type_hpp

#include "sequence.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <mpark/variant.hpp>

#include <pybind11/pybind11.h>

#include <any>
#include <chrono>

using BlocksciTypeVariant = mpark::variant<
	blocksci::Block,
	blocksci::Transaction,
	blocksci::Input,
	blocksci::Output,
	blocksci::AnyScript,
	blocksci::EquivAddress,
	blocksci::Cluster,
	blocksci::TaggedCluster,
	blocksci::TaggedAddress,
	blocksci::script::Pubkey,
	blocksci::script::PubkeyHash,
	blocksci::script::WitnessPubkeyHash,
	blocksci::script::MultisigPubkey,
	blocksci::script::Multisig,
	blocksci::script::ScriptHash,
	blocksci::script::WitnessScriptHash,
	blocksci::script::OpReturn,
	blocksci::script::Nonstandard,
	blocksci::script::WitnessUnknown,
	blocksci::AddressType::Enum,
	int64_t,
	bool,
	std::chrono::system_clock::time_point,
	blocksci::uint256,
	blocksci::uint160,
	pybind11::bytes,
	pybind11::list,
	std::string
>;

struct BlocksciType {
	BlocksciTypeVariant var;

	template <typename U, std::enable_if_t<std::is_convertible<U, BlocksciTypeVariant>::value> * = nullptr>
	explicit BlocksciType(U && t) : var(std::forward<U>(t)) {}

	BlocksciType(const BlocksciType &val) = default;
	BlocksciType(BlocksciType && val) = default;

	std::any toAny() const {
		return mpark::visit([&](auto &r) -> std::any { return r; }, var);
	}
};


#endif /* blocksci_type_hpp */
