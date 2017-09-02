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

#include <sstream>

namespace blocksci {
    OutputPointer::OutputPointer(uint32_t txNum_, uint16_t outputNum_) : txNum(txNum_), outputNum(outputNum_) {}
    
    Transaction OutputPointer::getTransaction(const ChainAccess &access) const {
        return Transaction(access.createTx(txNum), txNum, access.getBlockHeight(txNum));
    }
    
    const Output &OutputPointer::getOutput(const ChainAccess &access) const {
        return access.createOutput(*this);
    }
    
    std::string OutputPointer::toString() const {
        std::stringstream ss;
        ss << "OutputPointer(tx_index_from=" << txNum << ", output_index_from=" << outputNum << ")";
        return ss.str();
    }
}
