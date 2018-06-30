//
//  utxo.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#ifndef utxo_hpp
#define utxo_hpp

#include <blocksci/address/address_types.hpp>
#include <blocksci/chain/output.hpp>

struct UTXO {
    uint64_t value;
    uint32_t txNum;
    blocksci::AddressType::Enum type;
    
    UTXO() = default;
    UTXO(uint64_t value, uint32_t txNum, blocksci::AddressType::Enum type);
};

#endif /* utxo_hpp */
