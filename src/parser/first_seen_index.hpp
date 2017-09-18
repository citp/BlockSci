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
    
    void maybeUpdate(const blocksci::Address &address, uint32_t txNum);
    
    void sawAddress(const blocksci::Address &address, const blocksci::OutputPointer &pointer) override;
    void revealedP2SH(blocksci::script::ScriptHash &scriptHash, const blocksci::ScriptAccess &scripts) override;
    
    void prepareUpdate(const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts) override;
    
public:
    FirstSeenIndex(const ParserConfiguration &config);
};

#endif /* first_seen_index_hpp */
