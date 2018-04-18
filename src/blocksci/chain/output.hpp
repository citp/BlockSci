//
//  output.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef output_hpp
#define output_hpp

#include <blocksci/blocksci_export.h>
#include "inout.hpp"
#include "inout_pointer.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/util/data_access.hpp>

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::Output> {
        size_t operator()(const blocksci::Output &output) const;
    };
} // namespace std

namespace blocksci {
    class BLOCKSCI_EXPORT Output {
        DataAccess *access;
        const Inout *inout;
        uint32_t spendingTxIndex;
        
        friend size_t std::hash<Output>::operator()(const Output &) const;
    public:
        OutputPointer pointer;
        BlockHeight blockHeight;
        Output(const OutputPointer &pointer_, BlockHeight blockHeight_, const Inout &inout_, DataAccess &access_) :
        access(&access_), inout(&inout_), pointer(pointer_), blockHeight(blockHeight_) {
            assert(pointer.isValid(access_.chain));
            if (inout->getLinkedTxNum() < access->chain.maxLoadedTx()) {
                spendingTxIndex = inout->getLinkedTxNum();
            } else {
                spendingTxIndex = 0;
            }
        }
        
        Output(const OutputPointer &pointer_, DataAccess &access_) :
        Output(pointer_, access_.chain.getBlockHeight(pointer_.txNum), access_.chain.getTx(pointer_.txNum)->getOutput(pointer_.inoutNum), access_) {}
        
        DataAccess &getAccess() const {
            return *access;
        }
        
        ranges::optional<uint32_t> getSpendingTxIndex() const {
            return spendingTxIndex > 0 ? ranges::optional<uint32_t>{spendingTxIndex} : ranges::nullopt;
        }
        
        uint32_t txIndex() const {
            return pointer.txNum;
        }

        uint32_t outputIndex() const {
            return pointer.inoutNum;
        }
        
        Transaction transaction() const;
        Block block() const;
        
        bool isSpent() const {
            return spendingTxIndex > 0u;
        }
        
        bool operator==(const Output &other) const {
            return pointer == other.pointer;
        }

        bool operator!=(const Output &other) const {
            return pointer != other.pointer;
        }
        
        bool operator==(const Inout &other) const {
            return *inout == other;
        }
        
        blocksci::AddressType::Enum getType() const {
            return inout->getType();
        }
        
        Address getAddress() const {
            return {inout->getAddressNum(), inout->getType(), *access};
        }
        
        int64_t getValue() const {
            return inout->getValue();
        }

        std::string toString() const;
        ranges::optional<Transaction> getSpendingTx() const;
    };

    inline std::ostream &operator<<(std::ostream &os, const Output &output) { 
        return os << output.toString();
    }
} // namespace blocksci

namespace std {
    inline size_t hash<blocksci::Output>::operator()(const blocksci::Output &output) const {
        std::size_t seed = 819543;
        hash_combine(seed, output.pointer);
        return seed;
    }
} // namespace std

#endif /* output_hpp */
