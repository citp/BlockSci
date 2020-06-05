//
//  blocksci_type_converter.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/25/18.
//

#include "blocksci_type_converter.hpp"

#include <blocksci/chain/block_range.hpp>
#include <blocksci/scripts/script_variant.hpp>

blocksci::AnyScript BlockSciTypeConverter::operator()(const blocksci::Address &address) {
    return address.getScript();
}

RawRange<blocksci::Input> BlockSciTypeConverter::operator()(const blocksci::InputRange &val) {
    return ranges::any_view<blocksci::Input, random_access_sized>{val};
}

RawRange<blocksci::Output> BlockSciTypeConverter::operator()(const blocksci::OutputRange &val) {
    return ranges::any_view<blocksci::Output, random_access_sized>{val};
}

RawRange<blocksci::Block> BlockSciTypeConverter::operator()(const blocksci::BlockRange &val) {
    return ranges::any_view<blocksci::Block, random_access_sized>{val};
}