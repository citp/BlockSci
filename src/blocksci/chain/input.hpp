//
//  raw_input.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/2/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef raw_input_hpp
#define raw_input_hpp

#include "inout.hpp"

#include <cstdint>
#include <string>
#include <stdio.h>


namespace blocksci {
    class ChainAccess;
    struct Output;
    
    struct Input : public Inout {
        using Inout::Inout;
        
        Output matchedOutput(uint32_t txIndex) const;
        
        std::string toString() const;
        
        uint32_t spentTxIndex() const {
            return linkedTxNum;
        }
        
        Transaction getSpentTx(const ChainAccess &access) const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Transaction getSpentTx() const;
        #endif
    };
}

std::ostream &operator<<(std::ostream &os, blocksci::Input const &input);

#endif /* raw_input_hpp */
