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
#include <blocksci/scripts/script_types.hpp>

#include <boost/optional.hpp>

#include <memory>
#include <stdio.h>

namespace blocksci {
    struct Transaction;
    struct AddressPointer;
    struct Address;
    struct OutputPointer;
    class ChainAccess;
    
    struct Output : public Inout {
        Output(const Inout &other) : Inout(other) {}
        
        uint32_t getSpendingTxIndex(const ChainAccess &access) const;
        
        bool isSpent(const ChainAccess &access) const {
            return getSpendingTxIndex(access) != 0;
        }
        
        std::string toString() const;
        
        bool operator==(const Output& other) const;
        
        bool operator!=(const Output& other) const {
            return ! operator==(other);
        }
        
        boost::optional<Transaction> getSpendingTx(const ChainAccess &access) const;
        
        AddressPointer getAddressPointer() const;
        
        
        // Requires DataAccess
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        
        static const Output &create(const OutputPointer &pointer);
        
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
