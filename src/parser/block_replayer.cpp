//
//  block_replayer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/22/17.
//
//

#include "block_replayer.hpp"

#include "blockchain_state.hpp"
#include "address_writer.hpp"
#include "script_output.hpp"
#include "chain_index.hpp"
#include "preproccessed_block.hpp"
#include "utilities.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/chain/chain_access.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>

#include <boost/variant/variant_fwd.hpp>

struct ProcessOutputVisitorReplay : public boost::static_visitor<blocksci::Address> {
    const BlockchainState &state;
    ProcessOutputVisitorReplay(const BlockchainState &state_) : state(state_) {}
    template <blocksci::AddressType::Enum type>
    blocksci::Address operator()(ScriptOutput<type> &scriptOutput) const {
        std::pair<blocksci::Address, bool> processed = checkAddressNum(scriptOutput, state);
        if (processed.second) {
            scriptOutput.checkOutput(state);
        }
        return processed.first;
    }
};

template<blocksci::AddressType::Enum type>
struct ScriptInputFunctor {
    static void f(const InputInfo &info, const RawTransaction &tx, const BlockchainState &state, const AddressWriter &addressWriter) {
        ScriptInput<type> input(info, tx, addressWriter);
        input.checkInput(info, tx, state, addressWriter);
    }
};

void processInputVisitor(const InputInfo &info, const RawTransaction &tx, const BlockchainState &state, const AddressWriter &addressWriter) {
    auto &type = info.address.type;
    
    static constexpr auto table = blocksci::make_dynamic_table<ScriptInputFunctor>();
    static constexpr std::size_t size = blocksci::AddressType::all.size();
    
    auto index = static_cast<size_t>(type);
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    table[index](info, tx, state, addressWriter);
}

#ifdef BLOCKSCI_FILE_PARSER
void replayBlock(const FileParserConfiguration &config, uint32_t blockNum) {
    ECCVerifyHandle handle;
    ChainIndex index(config);
    auto chain = index.generateChain(blockNum);
    auto block = chain.back();
    auto blockPath = config.pathForBlockFile(block.nFile);
    boost::iostreams::mapped_file blockFile(blockPath, boost::iostreams::mapped_file::readonly);
    if (!blockFile.is_open()) {
        std::cout << "Error: Failed to open block file " << blockPath << "\n";
        return;
    }
    const char *startPos = blockFile.const_data() + block.nDataPos;
    constexpr size_t blockHeaderSize = 80;
    startPos += blockHeaderSize;
    uint32_t txCount = readVariableLengthInteger(&startPos);
    blocksci::uint256 nullHash;
    nullHash.SetNull();
    
    std::vector<unsigned char> coinbase;
    
    BlockchainState blockchainState(config);
    AddressWriter addressWriter(config);
    
    const BlockchainState &state = blockchainState;
    const AddressWriter &writer = addressWriter;
    blocksci::ChainAccess currentChain(config, false, 0);
    
    ProcessOutputVisitorReplay outputVisitor{state};
    
    auto realBlock = currentChain.getBlock(blockNum);
    for (uint32_t txNum = 0; txNum < txCount; txNum++) {
        auto realTx = realBlock.getTx(currentChain, txNum);
        
        RawTransaction tx;
        tx.load(&startPos);
        
        if (tx.inputs.size() == 1 && tx.inputs[0].rawOutputPointer.hash == nullHash) {
            coinbase.assign(tx.inputs[0].scriptBegin, tx.inputs[0].scriptEnd);
            tx.inputs.clear();
        }
        
        for (auto &output : tx.outputs) {
            auto address = boost::apply_visitor(outputVisitor, output.scriptOutput);
        }
        
        uint16_t j = 0;
        for (auto &input : tx.inputs) {
            auto &realInput = realTx.inputs()[j];
            InputInfo info{realInput.getAddress(), j, input.scriptBegin, input.scriptEnd};
            processInputVisitor(info, tx, state, writer);
            j++;
        }
    }
}
#endif
