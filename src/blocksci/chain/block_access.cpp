//
//  block_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "block.hpp"
#include "data_access.hpp"
#include "transaction.hpp"
#include "output.hpp"
#include "transaction_iterator.hpp"

namespace blocksci {
    const Block &Block::create(uint32_t height) {
        return create(*DataAccess::Instance().chain, height);
    }
    
    std::vector<unsigned char> Block::getCoinbase() const {
        return getCoinbase(*DataAccess::Instance().chain);
    }
    
    Block::const_iterator Block::begin() const {
        return begin(*DataAccess::Instance().chain);
    }
    
    Block::const_iterator Block::end() const {
        return end(*DataAccess::Instance().chain);
    }
    
    Block::TransactionRange Block::txes() const {
        return txes(*DataAccess::Instance().chain);
    }
    
    Block::value_type Block::operator[](uint32_t txNum) const {
        return getTx(*DataAccess::Instance().chain, txNum);
    }
    
    Transaction Block::coinbaseTx() const {
        return coinbaseTx(*DataAccess::Instance().chain);
    }
    
    
    bool isSegwit(const Block &block) {
        auto &instance = DataAccess::Instance();
        return isSegwit(block, *instance.chain, *instance.scripts);
    }
    
    size_t sizeBytes(const Block &block) {
        return sizeBytes(block, *DataAccess::Instance().chain);
    }
    
    uint64_t totalOut(const Block &block) {
        return totalOut(block, *DataAccess::Instance().chain);
    }
    
    uint64_t totalIn(const Block &block) {
        return totalIn(block, *DataAccess::Instance().chain);
    }
    
    uint64_t totalOutAfterHeight(const Block &block, uint32_t height) {
        return totalOutAfterHeight(block, height, *DataAccess::Instance().chain);
    }
    
    std::vector<uint64_t> fees(const Block &block) {
        return fees(block, *DataAccess::Instance().chain);
    }
    
    std::vector<double> feesPerByte(const Block &block) {
        return feesPerByte(block, *DataAccess::Instance().chain);
    }
    
    std::vector<const Output *> getUnspentOutputs(const Block &block) {
        return getUnspentOutputs(block, *DataAccess::Instance().chain);
    }
    
    std::vector<const Output *> getOutputsSpentByHeight(const Block &block, uint32_t height) {
        return getOutputsSpentByHeight(block, height, *DataAccess::Instance().chain);
    }
    
    uint64_t getTotalSpentOfAge(const Block &block, uint32_t age) {
        return getTotalSpentOfAge(block, *DataAccess::Instance().chain, age);
    }
    
    std::vector<uint64_t> getTotalSpentOfAges(const Block &block, uint32_t maxAge) {
        return getTotalSpentOfAges(block, *DataAccess::Instance().chain, maxAge);
    }
    
    std::unordered_map<AddressType::Enum, int64_t> netAddressTypeValue(const Block &block) {
        return netAddressTypeValue(block, *DataAccess::Instance().chain);
    }

    std::unordered_map<std::string, int64_t> netFullTypeValue(const Block &block) {
        return netFullTypeValue(block, *DataAccess::Instance().chain, *DataAccess::Instance().scripts);
    }
}
