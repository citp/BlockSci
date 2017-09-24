//
//  address_traverser.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/29/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_traverser.hpp"
#include "parser_configuration.hpp"

#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/transaction_iterator.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/address/address_types.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/scripts/script_access.hpp>

AddressTraverser::AddressTraverser(const ParserConfiguration &config_, const std::string &resultName) : ParserIndex(config_, resultName) {
}


void AddressTraverser::processTx(const blocksci::ChainAccess &, const blocksci::ScriptAccess &scripts, const blocksci::Transaction &tx) {
    uint16_t outputNum = 0;
    for (auto &output : tx.outputs()) {
        auto address = output.getAddress();
        blocksci::OutputPointer pointer{tx.txNum, outputNum};
        std::function<bool(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &a) {
            sawAddress(a, pointer);
            // If address is p2sh then ignore the wrapped address if it was revealed after this transaction
            if (scriptType(a.type) == blocksci::ScriptType::Enum::SCRIPTHASH) {
                auto p2sh = blocksci::script::ScriptHash{scripts, a.addressNum};
                if (p2sh.txRevealed == 0 || tx.txNum < p2sh.txRevealed) {
                    return false;
                }
            }
            return true;
        };
        visit(address, visitFunc, scripts);
        outputNum++;
    }
}
