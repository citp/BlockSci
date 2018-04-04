//
//  blocksci_fwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/17/17.
//

#ifndef blocksci_fwd_h
#define blocksci_fwd_h

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/cluster/cluster_fwd.hpp>

namespace blocksci {
    using TxIndex = uint32_t;
    
    struct DataConfiguration;
    class DataAccess;
    class uint256;
    class uint160;
    class HashIndex;
    struct State;
}

#endif /* blocksci_fwd_h */
