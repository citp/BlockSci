//
//  output_proxy_range_map_sequence.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "output_proxy_py.hpp"
#include "proxy/range_map_sequence.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

void addOutputProxyMethodsRangeMapSequence(AllProxyClasses<blocksci::Output> &cls) {
	setupRangesMapSequenceProxy(cls);
}
