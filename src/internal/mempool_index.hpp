//
//  mempool_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/12/18.
//

#ifndef mempool_index_hpp
#define mempool_index_hpp

#include "file_mapper.hpp"

#include <range/v3/algorithm/upper_bound.hpp>
#include <range/v3/view/transform.hpp>

#include <wjfilesystem/path.h>

#include <chrono>

namespace blocksci {
    
    struct MempoolRecord {
        time_t time;
    };
    
    struct BlockRecord {
        time_t observationTime;
    };
    
    struct TimestampIndex {
        FixedSizeFileMapper<MempoolRecord> timestampFile;
        uint32_t firstTxIndex;
        
        explicit TimestampIndex(const filesystem::path &path, uint32_t firstTxIndex_) : timestampFile(path), firstTxIndex(firstTxIndex_) {}
        
        ranges::optional<std::chrono::system_clock::time_point> getTime(uint32_t index) const {
            auto record = getTimestamp(index);
            if (record.has_value()) {
                return std::chrono::system_clock::from_time_t(record.value());
            } else {
                return ranges::nullopt;
            }
        }

        ranges::optional<time_t> getTimestamp(uint32_t index) const {
            auto record = timestampFile[index - firstTxIndex];
            if (record->time > 1) {
                return record->time;
            } else {
                return ranges::nullopt;
            }
        }
        
        bool observed(uint32_t index) const {
            auto record = timestampFile[index - firstTxIndex];
            if (record->time > 0) {
                return true;
            } else {
                return false;
            }
        }
        
        bool contains(uint32_t index) const {
            return index >= firstTxIndex && (index - firstTxIndex) < timestampFile.size();
        }
        
        void reload() {
            timestampFile.reload();
        }
    };
    
    struct BlocktimeIndex {
        FixedSizeFileMapper<BlockRecord> timestampFile;
        int firstBlockNum;
        
        explicit BlocktimeIndex(const filesystem::path &path, int firstBlockNum_) : timestampFile(path), firstBlockNum(firstBlockNum_) {}
        
        ranges::optional<std::chrono::system_clock::time_point> getTime(OffsetType index) const {
            auto record = getTimestamp(index);
            if (record) {
                return std::chrono::system_clock::from_time_t(record.value());
            } else {
                return ranges::nullopt;
            }
        }

        ranges::optional<time_t> getTimestamp(OffsetType index) const {
            assert(index - firstBlockNum >= 0);
            auto record = timestampFile[index - firstBlockNum];
            if (record->observationTime > 0) {
                return record->observationTime;
            } else {
                return ranges::nullopt;
            }
        }
        
        bool contains(int index) const {
            return index >= firstBlockNum && (index - firstBlockNum) < static_cast<int>(timestampFile.size());
        }
        
        void reload() {
            timestampFile.reload();
        }
    };

    /** Provides access to the mempool index, which stores the timestamp of when a transaction has been
     * first seen. Only relevant when BlockSci's mempool_recorder is enabled (= running).
     *
     * Directory: mempool/
     */
    class MempoolIndex {
        filesystem::path baseDirectory;
        std::vector<TimestampIndex> timestampFiles;
        std::vector<BlocktimeIndex> blockTimeFiles;
        
        void setup() {
            timestampFiles.clear();
            FixedSizeFileMapper<uint32_t> txRecordingPositions{txIndexFilePath(baseDirectory)};
            for (OffsetType i = 0; i < static_cast<OffsetType>(txRecordingPositions.size()); i++) {
                timestampFiles.emplace_back(nthTxFilePath(baseDirectory, i), *txRecordingPositions[i]);
            }
            blockTimeFiles.clear();
            FixedSizeFileMapper<int> blockRecordingPositions{blockIndexFilePath(baseDirectory)};
            for (OffsetType i = 0; i < static_cast<OffsetType>(blockRecordingPositions.size()); i++) {
                blockTimeFiles.emplace_back(nthBlockFilePath(baseDirectory, i), *blockRecordingPositions[i]);
            }
        }
        
        static filesystem::path txIndexFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"tx_index";
        }
        
        static filesystem::path blockIndexFilePath(const filesystem::path &baseDirectory) {
            return baseDirectory/"block_index";
        }
        
        static filesystem::path nthTxFilePath(const filesystem::path &baseDirectory, int64_t i) {
            return baseDirectory/(std::to_string(i) + "_tx");
        }
        
        static filesystem::path nthBlockFilePath(const filesystem::path &baseDirectory, int64_t i) {
            return baseDirectory/(std::to_string(i) + "_block");
        }
        
    public:
        explicit MempoolIndex(filesystem::path baseDirectory_) :  baseDirectory(std::move(baseDirectory_)) {
            setup();
        }
        
        ranges::optional<std::reference_wrapper<const TimestampIndex>> selectPossibleTxRecording(uint32_t txIndex) const {
            if (timestampFiles.size() > 0) {
                auto it = ranges::upper_bound(timestampFiles, txIndex, ranges::less(), &TimestampIndex::firstTxIndex);
                if (it != timestampFiles.begin()) {
                    --it;
                }
                if (it->contains(txIndex)) {
                    return std::ref(*it);
                }
            }
            return ranges::nullopt;
        }
        
        ranges::optional<std::reference_wrapper<const BlocktimeIndex>> selectPossibleBlockRecording(int height) const {
            if (blockTimeFiles.size() > 0) {
                auto it = ranges::upper_bound(blockTimeFiles, height, ranges::less(), &BlocktimeIndex::firstBlockNum);
                if (it != blockTimeFiles.begin()) {
                    --it;
                }
                if (it->contains(height)) {
                    return std::ref(*it);
                }
            }
            return ranges::nullopt;
        }

        ranges::optional<std::chrono::system_clock::time_point> getTxTime(uint32_t index) const {
            auto possibleFile = selectPossibleTxRecording(index);
            if (possibleFile) {
                return (*possibleFile).get().getTime(index);
            } else {
                return ranges::nullopt;
            }
        }

        ranges::optional<time_t> getTxTimestamp(uint32_t index) const {
            auto possibleFile = selectPossibleTxRecording(index);
            if (possibleFile) {
                return (*possibleFile).get().getTimestamp(index);
            } else {
                return ranges::nullopt;
            }
        }
        
        bool observed(uint32_t index) const {
            auto possibleFile = selectPossibleTxRecording(index);
            if (possibleFile) {
                return (*possibleFile).get().observed(index);
            } else {
                return false;
            }
        }
        
        ranges::optional<std::chrono::system_clock::time_point> getBlockTime(int height) const {
            auto possibleFile = selectPossibleBlockRecording(height);
            if (possibleFile) {
                return (*possibleFile).get().getTime(height);
            } else {
                return ranges::nullopt;
            }
        }

        ranges::optional<time_t> getBlockTimestamp(int height) const {
            auto possibleFile = selectPossibleBlockRecording(height);
            if (possibleFile) {
                return (*possibleFile).get().getTimestamp(height);
            } else {
                return ranges::nullopt;
            }
        }
        
        void reload() {
            for (auto &file : timestampFiles) {
                file.reload();
            }
            setup();
        }
    };
} // namespace blocksci

#endif /* mempool_index_hpp */
