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

template<>
struct ParserIndexInfo<AddressDB> {
    static constexpr bool processesScript(blocksci::EquivAddressType::Enum type) {
        return type == blocksci::EquivAddressType::MULTISIG;
    }
};

class AddressDB : public ParserIndex<AddressDB> {
    blocksci::AddressIndex db;
    
public:
    
    
    
    AddressDB(const ParserConfigurationBase &config, const std::string &path);
    
    void processTx(const blocksci::Transaction &tx, const blocksci::ScriptAccess &scripts);
    template<blocksci::EquivAddressType::Enum type>
    void processScript(uint32_t, const blocksci::ChainAccess &, const blocksci::ScriptAccess &);
    
    void rollback(const blocksci::State &state);
    void tearDown() override;
};

template<>
inline void AddressDB::processScript<blocksci::EquivAddressType::MULTISIG>(uint32_t equivNum, const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts) {
    blocksci::script::Multisig multisig(scripts, equivNum);
    for (const auto &address : multisig.addresses) {
        db.addAddressNested(address, blocksci::EquivAddress{equivNum, blocksci::EquivAddressType::MULTISIG});
    }
}

#endif /* address_db_h */
