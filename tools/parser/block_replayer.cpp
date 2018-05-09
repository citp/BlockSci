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

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/filesystem/fstream.hpp>

bool isSegwitMarker(const blocksci::RawTransaction &tx, blocksci::ScriptAccess &scripts) {
    for (uint16_t i = 0; i < tx.outputCount; i++) {
        auto output = tx.getOutput(tx.outputCount - i - 1);
        if (output.getType() == blocksci::AddressType::NULL_DATA) {
            auto nulldata = scripts.getScriptData<blocksci::DedupAddressType::NULL_DATA>(output.getAddressNum());
            auto data = nulldata->getData();
            uint32_t startVal;
            std::memcpy(&startVal, data.c_str(), sizeof(startVal));
            if (startVal == 0xaa21a9ed) {
                return true;
            }
        }
    }
    return false;
}

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
    reader.readVariableLengthInteger();
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    
    std::vector<unsigned char> coinbase;
    
    HashIndexCreator hashDb(config, config.dataConfig.hashIndexFilePath());
    AddressState addressState{config.addressPath(), hashDb};
    blocksci::ChainAccess chainAccess{config.dataConfig.chainDirectory(), config.dataConfig.blocksIgnored, config.dataConfig.errorOnReorg};
    blocksci::ScriptAccess scripts{config.dataConfig.scriptsDirectory()};
    
    auto blockPtr = chainAccess.getBlock(blockNum);
    auto firstTxNum = blockPtr->firstTxIndex;
    auto segwit = isSegwitMarker(*chainAccess.getTx(firstTxNum), scripts);
    for (uint32_t txNum = firstTxNum; txNum < firstTxNum + blockPtr->txCount; txNum++) {
        auto realTx = chainAccess.getTx(txNum);
        RawTransaction tx;
        tx.load(reader, txNum, blockNum, segwit);
        
        if (tx.inputs.size() == 1 && tx.inputs[0].rawOutputPointer.hash == nullHash) {
            auto scriptView = tx.inputs[0].getScriptView();
            coinbase.assign(scriptView.begin(), scriptView.end());
            tx.inputs.clear();
        }
        
        uint16_t i = 0;
        for (auto &input : tx.inputs) {
            auto &realInput = realTx->getInput(i);
            auto address = blocksci::RawAddress{realInput.getAddressNum(), realInput.getType()};
            InputView inputView(i, tx.txNum, input.witnessStack, tx.isSegwit);
            AnySpendData spendData(address, scripts);
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
