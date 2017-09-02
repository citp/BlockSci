//
//  inout.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/28/17.
//
//

#include "inout.hpp"
#include "transaction.hpp"
#include "chain_access.hpp"
#include "scripts/address_pointer.hpp"

#include <sstream>

namespace blocksci {
    
    Inout::Inout(uint32_t linkedTxNum_, const AddressPointer &address, uint64_t value) : linkedTxNum(linkedTxNum_), toAddressNum(address.addressNum), other(0) {
        setValue(value);
        setType(address.type);
    }
    
    AddressPointer Inout::getAddressPointer() const {
        return AddressPointer(toAddressNum, getType());
    }
    
    bool Inout::operator==(const Inout& otherOutput) const {
        return linkedTxNum == otherOutput.linkedTxNum && toAddressNum == otherOutput.toAddressNum && other == otherOutput.other;
    }
    
//    const Output &Output::create(const ChainAccess &access, const OutputPointer &pointer) {
//        return access.createOutput(pointer);
//    }
//
}
