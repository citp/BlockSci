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
#include "output_spend_data.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>

#include <boost/variant/variant_fwd.hpp>

#ifdef BLOCKSCI_FILE_PARSER
void replayBlock(const ParserConfiguration<FileTag> &config, uint32_t blockNum) {
    ECCVerifyHandle handle;
    ChainIndex<FileTag> index;
    boost::filesystem::ifstream inFile(config.blockListPath(), std::ios::binary);
    if (!inFile.good()) {
        throw std::runtime_error("Can only replay block that has already been processed");
    }
    
    boost::archive::binary_iarchive ia(inFile);
    ia >> index;
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
    
    AddressState addressState_{config.addressPath()};
    
    const AddressState &addressState = addressState_;
    blocksci::ChainAccess currentChain(config, false, 0);
    blocksci::ScriptAccess scripts(config);
    
    auto realBlock = currentChain.getBlock(blockNum);
    auto segwit = isSegwit(realBlock, currentChain, scripts);
    for (uint32_t txNum = 0; txNum < txCount; txNum++) {
        auto realTx = realBlock.getTx(currentChain, txNum);
        
        RawTransaction tx;
        tx.load(reader, realTx.txNum, blockNum, segwit);
        
        if (tx.inputs.size() == 1 && tx.inputs[0].rawOutputPointer.hash == nullHash) {
            auto scriptView = tx.inputs[0].getScriptView();
            coinbase.assign(scriptView.begin(), scriptView.end());
            tx.inputs.clear();
        }
        
        uint16_t i = 0;
        for (auto &input : tx.inputs) {
            auto &realInput = realTx.inputs()[i];
            auto address = realInput.getAddress();
            InputView inputView(i, tx.txNum, input.witnessStack, tx.isSegwit);
            AnySpendData spendData(address.getScript(scripts), address.type, scripts);
            tx.scriptInputs.emplace_back(inputView, input.getScriptView(), tx, spendData);
            i++;
        }
        
        i = 0;
        for (auto &output : tx.outputs) {
            tx.scriptOutputs.emplace_back(output.getScriptView(), tx.isSegwit);
        }
        
        for (auto &scriptOutput : tx.scriptOutputs) {
            scriptOutput.check(addressState);
        }
        
        for (auto &scriptInput : tx.scriptInputs) {
            scriptInput.check(addressState);
        }
    }
}
#endif
