//
//  utxo.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#include "utxo.hpp"

UTXO::UTXO(uint64_t value_, uint32_t txNum_, blocksci::AddressType::Enum type_): value(value_), txNum(txNum_), type(type_) {}
