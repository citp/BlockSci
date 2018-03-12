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
    
public:
    
    
    
    AddressDB(const ParserConfigurationBase &config, const std::string &path);
    
    void processTx(const blocksci::Transaction &tx);
    
    template<blocksci::DedupAddressType::Enum type>
    void processScript(uint32_t, const blocksci::DataAccess &);
    
    void rollback(const blocksci::State &state);
    void tearDown() override;
};

template<>
inline void AddressDB::processScript<blocksci::DedupAddressType::MULTISIG>(uint32_t equivNum, const blocksci::DataAccess &access) {
    blocksci::script::Multisig multisig(equivNum, access);
    for (const auto &address : multisig.getAddresses()) {
        db.addAddressNested(address, blocksci::DedupAddress{equivNum, blocksci::DedupAddressType::MULTISIG});
    }
}

#endif /* address_db_h */
