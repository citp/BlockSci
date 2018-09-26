//
//  input_proxy_main.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "input_proxy_py.hpp"
#include "proxy_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>



void addInputProxyMethodsMain(AllProxyClasses<blocksci::Input> &cls) {
	cls.setupBasicProxy();
}
