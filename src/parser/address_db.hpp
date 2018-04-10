//
//  address_db.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/27/17.
//
//

#ifndef address_db_h
#define address_db_h

#include "parser_fwd.hpp"
#include "parser_index.hpp"

#include <blocksci/index/address_index.hpp>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/scripts/multisig_script.hpp>

#include <unordered_map>

class AddressDB;


template<blocksci::DedupAddressType::Enum type>
struct ParserIndexScriptInfo<AddressDB, type> : std::false_type {};

template<>
struct ParserIndexScriptInfo<AddressDB, blocksci::DedupAddressType::MULTISIG> : std::true_type {};

class AddressDB : public ParserIndex<AddressDB> {
    blocksci::AddressIndex db;
    
    static constexpr int cacheSize = 1000;
    
    std::vector<std::pair<blocksci::RawAddress, blocksci::OutputPointer>> outputCache;
    std::vector<std::pair<blocksci::RawAddress, blocksci::DedupAddress>> nestedCache;
    
    void clearNestedCache();
    void clearOutputCache();
public:
    
    AddressDB(const ParserConfigurationBase &config, const std::string &path);
    ~AddressDB();
    
    void processTx(const blocksci::RawTransaction *tx, uint32_t txNum, const blocksci::ChainAccess &chain, const blocksci::ScriptAccess &scripts);
    
    template<blocksci::DedupAddressType::Enum type>
    void processScript(uint32_t, const blocksci::ScriptAccess &);
    
    void addAddressNested(const blocksci::RawAddress &childAddress, const blocksci::DedupAddress &parentAddress);
    void addAddressOutput(const blocksci::RawAddress &address, const blocksci::OutputPointer &pointer);
    
    void rollback(const blocksci::State &state);
    void tearDown() override;
    
    void compact() {
        db.compactDB();
    }
};

template<>
inline void AddressDB::processScript<blocksci::DedupAddressType::MULTISIG>(uint32_t equivNum, const blocksci::ScriptAccess &scripts) {
    auto multisig = scripts.getScriptData<blocksci::DedupAddressType::MULTISIG>(equivNum);
    for (const auto &addressNum : multisig->addresses) {
        addAddressNested(blocksci::RawAddress{addressNum, blocksci::AddressType::MULTISIG_PUBKEY}, blocksci::DedupAddress{equivNum, blocksci::DedupAddressType::MULTISIG});
    }
}

#endif /* address_db_h */
