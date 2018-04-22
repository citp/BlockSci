//
//  range_conversion.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "range_conversion.hpp"

#include <blocksci/core/bitcoin_uint256.hpp>

std::array<char, 64> NumpyConverter<blocksci::uint256>::operator()(const blocksci::uint256 &val) {
    auto hexStr = val.GetHex();
    std::array<char, 64> ret;
    std::copy_n(hexStr.begin(), 64, ret.begin());
    return ret;
}
