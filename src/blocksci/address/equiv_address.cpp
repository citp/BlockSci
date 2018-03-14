//
//  equiv_address.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/13/18.
//

#include "equiv_address.hpp"

#include <blocksci/util/data_access.hpp>
#include <blocksci/util/hash.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>

using namespace blocksci;

namespace std
{
    size_t hash<blocksci::EquivAddress>::operator()(const blocksci::EquivAddress &equiv) const {
        std::size_t seed = 123954;
        for (const auto &address : equiv.getAddresses()) {
            hash_combine(seed, address);
        }
        return seed;
    }
}

void EquivAddress::validateAddresses() {
    if (!validated) {
        possibleAddresses = getAddresses();
        validated = true;
    }
}

std::unordered_set<Address> EquivAddress::getAddresses() const {
    if (!validated) {
        std::unordered_set<Address> newAddresses;
        for (const auto &address : possibleAddresses) {
            if (access.addressIndex->checkIfExists(address)) {
                newAddresses.insert(address);
            }
        }
        return newAddresses;
    }
    return possibleAddresses;
}

std::vector<OutputPointer> EquivAddress::getOutputPointers() const {
    std::vector<OutputPointer> outputs;
    for (const auto &address : possibleAddresses) {
        auto addrOuts = access.addressIndex->getOutputPointers(address);
        outputs.insert(outputs.end(), addrOuts.begin(), addrOuts.end());
    }
    return outputs;
}

uint64_t EquivAddress::calculateBalance(BlockHeight height) const {
    return blocksci::calculateBalance(getOutputPointers(), height, access);
}

std::vector<Output> EquivAddress::getOutputs() const {
    return blocksci::getOutputs(getOutputPointers(), access);
}

std::vector<Input> EquivAddress::getInputs() const {
    return blocksci::getInputs(getOutputPointers(), access);
}

std::vector<Transaction> EquivAddress::getTransactions() const {
    return blocksci::getTransactions(getOutputPointers(), access);
}

std::vector<Transaction> EquivAddress::getOutputTransactions() const {
    return blocksci::getOutputTransactions(getOutputPointers(), access);
}

std::vector<Transaction> EquivAddress::getInputTransactions() const {
    return blocksci::getInputTransactions(getOutputPointers(), access);
}


