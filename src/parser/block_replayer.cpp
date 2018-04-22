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
#include <blocksci/core/chain_access.hpp>
#include <blocksci/core/script_access.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/index/address_index.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/filesystem/fstream.hpp>

#ifdef BLOCKSCI_FILE_PARSER
void replayBlock(const ParserConfiguration<FileTag> &config, blocksci::BlockHeight blockNum) {
    blocksci::ECCVerifyHandle handle;
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
    SafeMemReader reader{blockPath.native()};
    reader.advance(block.nDataPos);
    reader.advance(sizeof(CBlockHeader));
    auto txCount = reader.readVariableLengthInteger();
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    
    std::vector<unsigned char> coinbase;
    
    HashIndexCreator hashDb(config, config.dataConfig.hashIndexFilePath().native());
    AddressState addressState{config.addressPath(), hashDb};
    blocksci::DataAccess access(config.dataConfig);
    
    auto realBlock = blocksci::Block(blockNum, access);
    auto segwit = isSegwit(realBlock);
    for (int txNum = 0; txNum < static_cast<int>(txCount); txNum++) {
        auto realTx = realBlock[txNum];
        
        RawTransaction tx;
        tx.load(reader, realTx.txNum, blockNum, segwit);
        
        if (tx.inputs.size() == 1 && tx.inputs[0].rawOutputPointer.hash == nullHash) {
            auto scriptView = tx.inputs[0].getScriptView();
            coinbase.assign(scriptView.begin(), scriptView.end());
            tx.inputs.clear();
        }
        
        uint16_t i = 0;
        for (auto &input : tx.inputs) {
            auto realInput = realTx.inputs()[i];
            auto address = realInput.getAddress();
            InputView inputView(i, tx.txNum, input.witnessStack, tx.isSegwit);
            AnySpendData spendData(address.getScript());
            tx.scriptInputs.emplace_back(inputView, input.getScriptView(), tx, spendData);
            i++;
        }
        
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
