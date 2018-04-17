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
    int64_t value;
    uint32_t txNum;
    blocksci::AddressType::Enum type;
    
    UTXO() = default;
    UTXO(int64_t value_, uint32_t txNum_, blocksci::AddressType::Enum type_) : value(value_), txNum(txNum_), type(type_) {}
};

#endif /* utxo_hpp */
