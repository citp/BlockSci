//
//  output.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef output_hpp
#define output_hpp

#include "inout.hpp"
#include <blocksci/address/address_types.hpp>

#include <boost/optional/optional_fwd.hpp>

#include <memory>

namespace blocksci {
    struct Transaction;
    struct Address;
    struct OutputPointer;
    struct Input;
    class ChainAccess;
    
    struct Output : public Inout {
        using Inout::Inout;
        
        Output(const Inout &other) : Inout(other) {}
        
        Input matchedInput(uint32_t txIndex) const;
        
        uint32_t getSpendingTxIndex(const ChainAccess &access) const;
        
        bool isSpent(const ChainAccess &access) const {
            return getSpendingTxIndex(access) != 0;
        }
        
        std::string toString() const;
        
        boost::optional<Transaction> getSpendingTx(const ChainAccess &access) const;
        
        // Requires DataAccess
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        
        uint32_t getSpendingTxIndex() const;
        boost::optional<Transaction> getSpendingTx() const;
        
        bool isSpent() const {
            return getSpendingTxIndex() != 0;
        }
        
        #endif
    };
}

std::ostream &operator<<(std::ostream &os, blocksci::Output const &output);

#endif /* output_hpp */
