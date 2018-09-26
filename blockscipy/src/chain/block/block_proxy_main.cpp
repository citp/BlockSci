//
//  block_proxy_main.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "block_proxy_py.hpp"
#include "proxy_py.hpp"

#include <blocksci/cluster/cluster.hpp>

void addBlockProxyMethodsMain(AllProxyClasses<blocksci::Block> &cls) {
	cls.setupBasicProxy();
}
