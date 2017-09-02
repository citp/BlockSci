//
//  address_traverser.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/29/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_traverser.hpp"

#include <blocksci/data_access.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/address_pointer.hpp>
#include <blocksci/scripts/address_types.hpp>
#include <blocksci/scripts/script_info.hpp>

void AddressTraverser::processTx(const blocksci::DataAccess &access, const blocksci::Transaction &tx) {
    uint16_t i = 0;
    for (auto &output : tx.outputs()) {
        auto pointer = output.getAddressPointer();
        sawAddress(pointer, tx.txNum);
        if (pointer.type == blocksci::ScriptType::Enum::MULTISIG) {
            auto address = pointer.getAddress(access.scripts);
            for (auto &nestedAddressPointer : address->nestedAddressPointers()) {
                if (nestedAddressPointer.addressNum != 0) {
                    sawAddress(nestedAddressPointer, tx.txNum);
                }
            }
        }
        i++;
    }
    
    for (auto &input : tx.inputs()) {
        if (input.getType() == blocksci::ScriptType::Enum::SCRIPTHASH) {
            auto pointer = input.getAddressPointer();
            auto address = pointer.getAddress(access.scripts);
            auto p2shAddress = dynamic_cast<blocksci::address::ScriptHash *>(address.get());
            processP2SHAddress(access.scripts, p2shAddress->wrappedAddressPointer, input.linkedTxNum, pointer.addressNum);
        }
    }
}

void AddressTraverser::processP2SHAddress(const blocksci::ScriptAccess &access, const blocksci::AddressPointer &pointer, uint32_t txNum, uint32_t p2shNum) {
    linkP2SHAddress(pointer, txNum, p2shNum);
    if (hasNestedAddresses(pointer.type)) {
        auto address = pointer.getAddress(access);
        for (auto &nestedAddressPointer : address->nestedAddressPointers()) {
            // This check shouldn't be necessary
            if (nestedAddressPointer.addressNum != 0) {
                processP2SHAddress(access, nestedAddressPointer, txNum, p2shNum);
            }
        }
    }
}
