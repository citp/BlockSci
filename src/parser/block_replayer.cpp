//
//  block_replayer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/22/17.
//
//

#include "block_replayer.hpp"

#include "address_state.hpp"
#include "utxo_state.hpp"
#include "address_writer.hpp"
#include "script_output.hpp"
#include "chain_index.hpp"
#include "preproccessed_block.hpp"
#include "safe_mem_reader.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>

#include <boost/variant/variant_fwd.hpp>

#ifdef BLOCKSCI_FILE_PARSER
void replayBlock(const ParserConfiguration<FileTag> &config, uint32_t blockNum) {
    ECCVerifyHandle handle;
    ChainIndex<FileTag> index(config);
    auto chain = index.generateChain(blockNum);
    auto block = chain.back();
    auto blockPath = config.pathForBlockFile(block.nFile);
    SafeMemReader reader{blockPath};
    reader.advance(block.nDataPos);
    reader.advance(sizeof(CBlockHeader));
    auto txCount = reader.readVariableLengthInteger();
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    
    std::vector<unsigned char> coinbase;
    
    AddressState addressState_{config};
    AddressWriter addressWriter(config);
    
    const AddressState &addressState = addressState_;
    const AddressWriter &writer = addressWriter;
    blocksci::ChainAccess currentChain(config, false, 0);
    blocksci::ScriptAccess scripts(config);
    
    auto realBlock = currentChain.getBlock(blockNum);
    auto segwit = isSegwit(realBlock, currentChain, scripts);
    for (uint32_t txNum = 0; txNum < txCount; txNum++) {
        auto realTx = realBlock.getTx(currentChain, txNum);
        
        RawTransaction tx;
        tx.load(reader, realTx.txNum, blockNum, segwit);
        
        if (tx.inputs.size() == 1 && tx.inputs[0].rawOutputPointer.hash == nullHash) {
            auto scriptBegin = tx.inputs[0].getScriptBegin();
            coinbase.assign(scriptBegin, scriptBegin + tx.inputs[0].getScriptLength());
            tx.inputs.clear();
        }
        
        for (auto &output : tx.outputs) {
            auto address = checkOutput(output.scriptOutput, addressState, writer);
        }
        
        uint16_t j = 0;
        for (auto &input : tx.inputs) {
            auto &realInput = realTx.inputs()[j];
            InputInfo info = input.getInfo(j, txNum, realInput.toAddressNum, segwit);
            checkInput(realInput.getType(), info, tx, addressState, writer);
            j++;
        }
    }
}
#endif
