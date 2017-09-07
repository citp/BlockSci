//
//  address_traverser.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/29/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_traverser.hpp"

#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/address_types.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/scripts/script_access.hpp>

void AddressTraverser::processTx(const blocksci::ScriptAccess &scripts, const blocksci::Transaction &tx) {
    uint16_t i = 0;
    for (auto &output : tx.outputs()) {
        auto address = output.getAddress();
        sawAddress(address, tx.txNum);
        if (address.type == blocksci::AddressType::Enum::MULTISIG) {
            auto script = address.getScript(scripts);
            for (auto &nestedAddressPointer : script->nestedAddresses()) {
                if (nestedAddressPointer.addressNum != 0) {
                    sawAddress(nestedAddressPointer, tx.txNum);
                }
            }
        }
        i++;
    }
    
    for (auto &input : tx.inputs()) {
        if (input.getType() == blocksci::AddressType::Enum::SCRIPTHASH) {
            auto address = input.getAddress();
            auto script = address.getScript(scripts);
            auto p2shAddress = dynamic_cast<blocksci::script::ScriptHash *>(script.get());
            processP2SHAddress(scripts, p2shAddress->wrappedAddress, input.spentTxIndex(), address.addressNum);
        }
    }
}

void AddressTraverser::processP2SHAddress(const blocksci::ScriptAccess &access, const blocksci::Address &address, uint32_t txNum, uint32_t p2shNum) {
    linkP2SHAddress(address, txNum, p2shNum);
    if (hasNestedAddresses(address.type)) {
        auto script = address.getScript(access);
        for (auto &nestedAddressPointer : script->nestedAddresses()) {
            // This check shouldn't be necessary
            if (nestedAddressPointer.addressNum != 0) {
                processP2SHAddress(access, nestedAddressPointer, txNum, p2shNum);
            }
        }
    }
}
