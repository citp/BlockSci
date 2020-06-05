//
//  chain_access.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/26/17.
//
//

#ifndef chain_access_hpp
#define chain_access_hpp

#include "file_mapper.hpp"
#include "exception.hpp"

#include <blocksci/core/bitcoin_uint256.hpp>
#include <blocksci/core/core_fwd.hpp>
#include <blocksci/core/raw_block.hpp>
#include <blocksci/core/raw_transaction.hpp>
#include <blocksci/core/typedefs.hpp>
#include <blocksci/core/transaction_data.hpp>

#include <wjfilesystem/path.h>

#include <algorithm>

namespace blocksci {

    /** Provides data access for blocks, transactions, inputs, and outputs.
     *
     * The files here represent the core data about blocks and transactions.
     * Data is stored in a hybrid column and row based structure.
     * The core tx_data.dat and tx_index.dat are the core data files containing most information that is
     * used about transactions, inputs, and outputs. The other files in this folder are column oriented storing
     * less frequently accessed values.
     *
     * Directory: chain/
     */
    class ChainAccess {
        /** Stores the serialized RawBlock data in the same order they occur in the blockchain, indexed by block height.
         *
         * File: chain/block.dat
         * Raw data format: [<RawBlock>, <RawBlock>, ...]
         */
        FixedSizeFileMapper<RawBlock> blockFile;

        /** Stores the coinbase data for every block, indexed by block height.
         *
         * Accessed by file offset that is stored for every block in Rawblock.coinbaseOffset
         *
         * File: chain/coinbases.dat
         * Raw data format: [uint32_t coinbaseLength, <coinbaseBytes>, uint32_t coinbaseLength, <coinbaseBytes>, ...]
         * Example: Genesis block of BTC contains "[...] The Times 03/Jan/2009 Chancellor on brink of second bailout for banks"
         */
        SimpleFileMapper<> blockCoinbaseFile;

        /** Stores the serialized RawTransaction data in the same order they occur in the blockchain, indexed by tx number.
         * Implemented as IndexedFileMapper as RawTransaction objects have variable length due to variable no. of inputs/outputs.
         *
         * The scripts and addresses are not stored in this file.
         * Instead, a scriptNum is stored that references to a scripts file, @link blocksci::ScriptAccess.
         *
         * Files: - chain/tx_data.dat: actual data
         *        - chain/tx_index.dat: index file that stores the offset for every transaction
         * Raw data format: [<RawTransaction>, <Inout list>, <RawTransaction>, <Inout list>, ...]
         */
        IndexedFileMapper<mio::access_mode::read, RawTransaction> txFile;

        /** Stores the version number that is stored in the first 4 bytes of every transaction on the blockchain, indexed by tx number.
         *
         * File: chain/tx_version.dat
         * Raw data format: [<int32_t versionNumberOfTx0>, <int32_t versionNumberOfTx1>, ...]
         */
        FixedSizeFileMapper<int32_t> txVersionFile;

        /** Stores the blockchain-wide number of the first input for every transaction, indexed by tx number.
         *
         * File(s): - chain/firstInput.dat
         * Raw data format: [<uint64_t firstInputNumberOfTx0>, <uint64_t firstInputNumberOfTx1>, ...]
         */
        FixedSizeFileMapper<uint64_t> txFirstInputFile;

        /** Stores the blockchain-wide number of the first output for every transaction, indexed by tx number.
         *
         * File: chain/firstOutput.dat
         * Raw data format: [<uint64_t firstOutputNumberOfTx0>, <uint64_t firstOutputNumberOfTx1>, ...]
         */
        FixedSizeFileMapper<uint64_t> txFirstOutputFile;

        /** Stores the tx-internal output number of the spent output for every input, indexed by blockchain-wide input number.
         *
         * File: chain/input_out_num.dat
         * Raw data format: [<uint16_t>, <uint16_t>, ...]
         */
        FixedSizeFileMapper<uint16_t> inputSpentOutputFile;

        /** Stores the blockchain field sequence number for every input, indexed by blockchain-wide input number.
         *
         * File: chain/sequence.dat
         * Raw data format: [<uint32_t sequenceFieldOfInput0>, <uint32_t sequenceFieldOfInput1>, ...]
         */
        FixedSizeFileMapper<uint32_t> sequenceFile;

        /** Stores a mapping of (tx number) to (tx hash), thus indexed by tx number.
         *
         * File: chain/tx_hashes.dat
         * Raw data format: [<uint64_t firstOutputNumberOfTx0>, <uint64_t firstOutputNumberOfTx1>, ...]
         */
        FixedSizeFileMapper<uint256> txHashesFile;

        /** Hash of the last loaded block */
        uint256 lastBlockHash;
        const uint256 *lastBlockHashDisk = nullptr;

        /** Number of the highest loaded block */
        BlockHeight maxHeight = 0;

        /** Number of the highest loaded transaction */
        uint32_t _maxLoadedTx = 0;

        /** Block height that BlockSci should load up to as a 1-indexed number
         * Eg. 10 loads blocks [0, 9], and -6 loads all but the last 6 blocks */
        BlockHeight blocksIgnored = 0;

        bool errorOnReorg = false;

        void reorgCheck() const {
            if (errorOnReorg && lastBlockHash != *lastBlockHashDisk) {
                throw ReorgException();
            }
        }

        void setup() {
            if (blocksIgnored <= 0) {
                maxHeight = static_cast<BlockHeight>(blockFile.size()) + blocksIgnored;
            } else {
                maxHeight = blocksIgnored;
            }
            if (maxHeight > BlockHeight(0)) {
                auto maxLoadedBlock = blockFile[static_cast<OffsetType>(maxHeight) - 1];
                lastBlockHash = maxLoadedBlock->hash;
                _maxLoadedTx = maxLoadedBlock->firstTxIndex + maxLoadedBlock->txCount;
                lastBlockHashDisk = &maxLoadedBlock->hash;
            } else {
                lastBlockHash.SetNull();
                _maxLoadedTx = 0;
                lastBlockHashDisk = nullptr;
            }

            if (_maxLoadedTx > txFile.size()) {
                std::stringstream ss;
                ss << "Block data corrupted. Tx file has " << txFile.size() << " transaction, but max tx to load is tx " << _maxLoadedTx;
                throw std::runtime_error(ss.str());
            }
        }

    public:
        explicit ChainAccess(const filesystem::path &baseDirectory, BlockHeight blocksIgnored, bool errorOnReorg) :
        blockFile(blockFilePath(baseDirectory)),
        blockCoinbaseFile(blockCoinbaseFilePath(baseDirectory)),
        txFile(txFilePath(baseDirectory)),
        txVersionFile(txVersionFilePath(baseDirectory)),
        txFirstInputFile(firstInputFilePath(baseDirectory)),
        txFirstOutputFile(firstOutputFilePath(baseDirectory)),
        inputSpentOutputFile(inputSpentOutNumFilePath(baseDirectory)),
        sequenceFile(sequenceFilePath(baseDirectory)),
        txHashesFile(txHashesFilePath(baseDirectory)),
        blocksIgnored(blocksIgnored),
        errorOnReorg(errorOnReorg) {
            setup();
        }

        static filesystem::path txFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"tx";
        }

        static filesystem::path txHashesFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"tx_hashes";
        }

        static filesystem::path blockFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"block";
        }

        static filesystem::path blockCoinbaseFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"coinbases";
        }

        static filesystem::path txVersionFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"tx_version";
        }

        static filesystem::path firstInputFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"firstInput";
        }

        static filesystem::path firstOutputFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"firstOutput";
        }

        static filesystem::path sequenceFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"sequence";
        }

        static filesystem::path inputSpentOutNumFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"input_out_num";
        }

        BlockHeight getBlockHeight(uint32_t txIndex) const {
            reorgCheck();
            if (errorOnReorg && txIndex >= _maxLoadedTx) {
                throw std::out_of_range("Transaction index out of range");
            }
            auto blockBegin = blockFile[0];
            auto blockEnd = blockFile[static_cast<OffsetType>(maxHeight) - 1] + 1;
            auto it = std::upper_bound(blockBegin, blockEnd, txIndex, [](uint32_t index, const RawBlock &b) {
                return index < b.firstTxIndex;
            });
            it--;
            return static_cast<BlockHeight>(std::distance(blockBegin, it));
        }

        const RawBlock *getBlock(BlockHeight blockHeight) const {
            reorgCheck();
            return blockFile[static_cast<OffsetType>(blockHeight)];
        }

        const uint256 *getTxHash(uint32_t index) const {
            reorgCheck();
            return txHashesFile[index];
        }

        const RawTransaction *getTx(uint32_t index) const {
            reorgCheck();
            return txFile.getData(index);
        }

        const int32_t *getTxVersion(uint32_t index) const {
            reorgCheck();
            return txVersionFile[index];
        }

        const uint32_t *getSequenceNumbers(uint32_t index) const {
            reorgCheck();
            return sequenceFile[static_cast<OffsetType>(*txFirstInputFile[index])];
        }

        const uint16_t *getSpentOutputNumbers(uint32_t index) const {
            reorgCheck();
            return inputSpentOutputFile[static_cast<OffsetType>(*txFirstInputFile[index])];
        }

        /** Get TxData object for given tx number */
        TxData getTxData(uint32_t index) const {
            reorgCheck();
            // Blockchain-wide number of first input for the given tx
            auto firstInputNum = static_cast<OffsetType>(*txFirstInputFile[index]);
            const uint16_t *inputsSpent = nullptr;
            const uint32_t *sequenceNumbers = nullptr;
            if (firstInputNum < inputSpentOutputFile.size()) {
                inputsSpent = inputSpentOutputFile[firstInputNum];
                sequenceNumbers = sequenceFile[firstInputNum];
            }
            return {                   // construct TxData object
                txFile.getData(index), // const RawTransaction *rawTx
                txVersionFile[index],  // const int32_t *version
                txHashesFile[index],   // const uint256 *hash
                inputsSpent,           // const uint16_t *spentOutputNums
                sequenceNumbers        // const uint32_t *sequenceNumbers
            };
        }

        size_t txCount() const {
            return _maxLoadedTx;
        }

        uint64_t inputCount() const {
            if (_maxLoadedTx > 0) {
                auto lastTx = getTx(_maxLoadedTx - 1);
                return *txFirstInputFile[_maxLoadedTx - 1] + lastTx->inputCount;
            } else {
                return 0;
            }
        }

        uint64_t outputCount() const {
            if (_maxLoadedTx > 0) {
                auto lastTx = getTx(_maxLoadedTx - 1);
                return *txFirstOutputFile[_maxLoadedTx - 1] + lastTx->outputCount;
            } else {
                return 0;
            }
        }

        BlockHeight blockCount() const {
            return maxHeight;
        }

        std::vector<unsigned char> getCoinbase(uint64_t offset) const {
            auto pos = blockCoinbaseFile.getDataAtOffset(static_cast<OffsetType>(offset));
            uint32_t coinbaseLength;
            std::memcpy(&coinbaseLength, pos, sizeof(coinbaseLength));
            uint64_t length = coinbaseLength;
            pos += sizeof(coinbaseLength);
            auto unsignedPos = reinterpret_cast<const unsigned char *>(pos);
            return std::vector<unsigned char>(unsignedPos, unsignedPos + length);
        }

        void reload() {
            blockFile.reload();
            blockCoinbaseFile.reload();
            txFile.reload();
            txFirstInputFile.reload();
            txFirstOutputFile.reload();
            txVersionFile.reload();
            inputSpentOutputFile.reload();
            txHashesFile.reload();
            sequenceFile.reload();
            setup();
        }
    };
} // namespace blocksci

#endif /* chain_access_hpp */
