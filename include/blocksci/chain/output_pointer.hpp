//
//  output_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef output_pointer_hpp
#define output_pointer_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/core/inout_pointer.hpp>

#include <string>
#include <vector>

namespace blocksci {
    class DataAccess;
    /** Contains data to uniquely identify one output using txNum and inoutNum */
    struct BLOCKSCI_EXPORT OutputPointer : public InoutPointer {
        using InoutPointer::InoutPointer;
        
        std::string toString() const;
    };
    
    std::vector<Input> BLOCKSCI_EXPORT getInputs(const std::vector<OutputPointer> &pointers, DataAccess &access);
    std::vector<Transaction> BLOCKSCI_EXPORT getTransactions(const std::vector<OutputPointer> &pointers, DataAccess &access);
    std::vector<Transaction> BLOCKSCI_EXPORT getOutputTransactions(const std::vector<OutputPointer> &pointers, DataAccess &access);
    std::vector<Transaction> BLOCKSCI_EXPORT getInputTransactions(const std::vector<OutputPointer> &pointers, DataAccess &access);
} // namespace blocksci

std::ostream BLOCKSCI_EXPORT &operator<<(std::ostream &os, const blocksci::OutputPointer &pointer);

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::OutputPointer> {
        size_t operator()(const blocksci::OutputPointer &pointer) const {
            std::size_t seed = 657434;
            blocksci::hash_combine(seed, static_cast<const blocksci::InoutPointer &>(pointer));
            return seed;
        }
    };
} // namespace std

#endif /* output_pointer_hpp */
