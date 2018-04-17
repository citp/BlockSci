//
//  performance.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/10/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef performance_hpp
#define performance_hpp

#include <blocksci/blocksci.hpp>

#include <unordered_map>

std::vector<int64_t> unspentSums1(blocksci::Blockchain &chain, int start, int stop);
std::vector<int64_t> unspentSums2(blocksci::Blockchain &chain, int start, int stop);

uint32_t maxSizeTx1(blocksci::Blockchain &chain, int start, int stop);
uint32_t maxSizeTx2(blocksci::Blockchain &chain, int start, int stop);

std::unordered_map<int64_t, int64_t> getOutputDistribution1(blocksci::Blockchain &chain, int start, int stop);
std::unordered_map<int64_t, int64_t> getOutputDistribution2(blocksci::Blockchain &chain, int start, int stop);

int64_t maxValOutput1(blocksci::Blockchain &chain, int start, int stop);
int64_t maxValOutput2(blocksci::Blockchain &chain, int start, int stop);

#endif /* performance_hpp */
