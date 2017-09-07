//
//  first_seen_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/29/17.
//
//

#ifndef first_seen_index_hpp
#define first_seen_index_hpp

#include "address_traverser.hpp"

#include <blocksci/scripts/script_type.hpp>
#include <blocksci/file_mapper.hpp>

#include <unordered_map>
#include <stdio.h>

struct ParserConfiguration;

class FirstSeenIndex : public AddressTraverser {
    std::unordered_map<blocksci::ScriptType::Enum, blocksci::FixedSizeFileMapper<uint32_t, boost::iostreams::mapped_file::readwrite>> files;
    
    void sawAddress(const blocksci::Address &pointer, uint32_t txNum) override;
    void linkP2SHAddress(const blocksci::Address &pointer, uint32_t txNum, uint32_t p2shNum) override;
    
public:
    FirstSeenIndex(const ParserConfiguration &config, const blocksci::ScriptAccess &access);
    
};

#endif /* first_seen_index_hpp */
