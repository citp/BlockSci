//
//  utxo.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#include "utxo.hpp"

UTXO::UTXO(const blocksci::Output &output_, blocksci::AddressType::Enum addressType_): output(output_), addressType(addressType_) {}
