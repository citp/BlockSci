//
//  output.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/6/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef output_hpp
#define output_hpp

#include "chain_access.hpp"
#include "inout_pointer.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/core/inout.hpp>
#include <blocksci/core/raw_transaction.hpp>

#include <blocksci/address/address.hpp>
#include <blocksci/util/data_access.hpp>

#include <sstream>

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
        mutable BlockHeight blockHeight = -1;
        friend size_t std::hash<Output>::operator()(const Output &) const;
    public:
        OutputPointer pointer;
        
        Output(const OutputPointer &pointer_, BlockHeight blockHeight_, const Inout &inout_, DataAccess &access_) :
        access(&access_), inout(&inout_), blockHeight(blockHeight_), pointer(pointer_) {
            assert(pointer.isValid(access_.getChain()));
            if (inout->getLinkedTxNum() < access->getChain().maxLoadedTx()) {
                spendingTxIndex = inout->getLinkedTxNum();
            } else {
                spendingTxIndex = 0;
            }
        }
        
        Output(const OutputPointer &pointer_, DataAccess &access_) :
        Output(pointer_, -1, access_.getChain().getTx(pointer_.txNum)->getOutput(pointer_.inoutNum), access_) {}
        
        DataAccess &getAccess() const {
            return *access;
        }
        
        BlockHeight getBlockHeight() const {
            if (blockHeight == -1) {
                blockHeight = access->getChain().getBlockHeight(pointer.txNum);
            }
            return blockHeight;
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

        std::string toString() const  {
            std::stringstream ss;
            ss << "TxOut(spending_tx_index=" << inout->getLinkedTxNum() << ", address=" << getAddress().toString() << ", value=" << inout->getValue() << ")";
            return ss.str();
        }
        
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
