//
//  output_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/18/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "output_pointer.hpp"
#include "chain_access.hpp"
#include "transaction.hpp"
#include "input.hpp"
#include "output.hpp"
#include "address/address.hpp"

#include "hash.hpp"

#include <sstream>

namespace blocksci {
    const Output &OutputPointer::getOutput(const ChainAccess &access) const {
        return access.getOutput(txNum, inoutNum);
    }
    
    Input OutputPointer::getInput(const ChainAccess &access) const {
        return getOutput(access).matchedInput(txNum);
    }
    
    std::string OutputPointer::toString() const {
        std::stringstream ss;
        ss << "OutputPointer(tx_index_from=" << txNum << ", output_index_from=" << inoutNum << ")";
        return ss.str();
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::OutputPointer &pointer) {
    os << pointer.toString();
    return os;
}

namespace std
{
    size_t hash<blocksci::OutputPointer>::operator()(const blocksci::OutputPointer &pointer) const {
        std::size_t seed = 41352363;
        hash_combine(seed, pointer.txNum);
        hash_combine(seed, pointer.inoutNum);
        return seed;
    }
}
