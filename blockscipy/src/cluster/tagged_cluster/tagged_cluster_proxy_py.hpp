//
//  tagged_cluster_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef tagged_cluster_proxy_py_h
#define tagged_cluster_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/cluster/cluster.hpp>

void addTaggedClusterProxyMethodsMain(AllProxyClasses<blocksci::TaggedCluster> &cls);
void addTaggedClusterProxyMethodsRange(AllProxyClasses<blocksci::TaggedCluster> &cls);

void addTaggedClusterProxyMethods(AllProxyClasses<blocksci::TaggedCluster> &cls);

#endif /* tagged_cluster_proxy_py_h */
