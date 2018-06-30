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

#include <stdio.h>

std::vector<uint64_t> unspentSums1(blocksci::Blockchain &chain, uint32_t start, uint32_t stop);
std::vector<uint64_t> unspentSums2(blocksci::Blockchain &chain, uint32_t start, uint32_t stop);

uint32_t maxSizeTx1(blocksci::Blockchain &chain, uint32_t start, uint32_t stop);
uint32_t maxSizeTx2(blocksci::Blockchain &chain, uint32_t start, uint32_t stop);

std::unordered_map<uint64_t, uint64_t> getOutputDistribution1(blocksci::Blockchain &chain, uint32_t start, uint32_t stop);
std::unordered_map<uint64_t, uint64_t> getOutputDistribution2(blocksci::Blockchain &chain, uint32_t start, uint32_t stop);

uint64_t maxValOutput1(blocksci::Blockchain &chain, uint32_t start, uint32_t stop);
uint64_t maxValOutput2(blocksci::Blockchain &chain, uint32_t start, uint32_t stop);

#endif /* performance_hpp */
