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

#include <stdio.h>

struct UTXO {
    blocksci::Output output;
    blocksci::AddressType::Enum addressType;
    
    UTXO() = default;
    UTXO(const blocksci::Output &output, blocksci::AddressType::Enum addressType);
};

#endif /* utxo_hpp */
