//
//  input_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "input_pointer.hpp"
#include "chain_access.hpp"
#include "input.hpp"
#include "output.hpp"

#include "hash.hpp"

#include <sstream>

namespace blocksci {
    const Input &InputPointer::getInput(const ChainAccess &access) const {
        return access.getInput(txNum, inoutNum);
    }
    
    Output InputPointer::getOutput(const ChainAccess &access) const {
        return getInput(access).matchedOutput(txNum);
    }
    
    std::string InputPointer::toString() const {
        std::stringstream ss;
        ss << "InputPointer(tx_index=" << txNum << ", input_index=" << inoutNum << ")";
        return ss.str();
    }
}

namespace std
{
    size_t hash<blocksci::InputPointer>::operator()(const blocksci::InputPointer &pointer) const {
        std::size_t seed = 41352363;
        hash_combine(seed, pointer.txNum);
        hash_combine(seed, pointer.inoutNum);
        return seed;
    }
}
