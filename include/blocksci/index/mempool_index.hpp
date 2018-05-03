//
//  mempool_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/12/18.
//

#ifndef mempool_index_hpp
#define mempool_index_hpp

#include <blocksci/blocksci_export.h>

#include <blocksci/core/file_mapper.hpp>

#include <range/v3/algorithm/upper_bound.hpp>
#include <range/v3/view/transform.hpp>

#include <chrono>

namespace blocksci {
    
    struct BLOCKSCI_EXPORT MempoolRecord {
        time_t time;
    };
    
    struct BLOCKSCI_EXPORT BlockRecord {
        time_t observationTime;
    };
    
    struct BLOCKSCI_EXPORT TimestampIndex {
        FixedSizeFileMapper<MempoolRecord> timestampFile;
        uint32_t firstTxIndex;
        
        explicit TimestampIndex(const std::string &path, uint32_t firstTxIndex_) : timestampFile(path), firstTxIndex(firstTxIndex_) {}
        
        ranges::optional<std::chrono::system_clock::time_point> getTimestamp(uint32_t index) const {
            auto record = timestampFile[index - firstTxIndex];
            if (record->time > 1) {
                return std::chrono::system_clock::from_time_t(record->time);
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
    
    struct BLOCKSCI_EXPORT BlocktimeIndex {
        FixedSizeFileMapper<BlockRecord> timestampFile;
        int firstBlockNum;
        
        explicit BlocktimeIndex(const std::string &path, int firstBlockNum_) : timestampFile(path), firstBlockNum(firstBlockNum_) {}
        
        ranges::optional<std::chrono::system_clock::time_point> getTimestamp(int index) const {
            assert(index - firstBlockNum >= 0);
            auto record = timestampFile[static_cast<size_t>(index - firstBlockNum)];
            if (record->observationTime > 0) {
                return std::chrono::system_clock::from_time_t(record->observationTime);
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
    
    class BLOCKSCI_EXPORT MempoolIndex {
        std::string baseDirectory;
        std::vector<TimestampIndex> timestampFiles;
        std::vector<BlocktimeIndex> blockTimeFiles;
        
        void setup() {
            timestampFiles.clear();
            FixedSizeFileMapper<uint32_t> txRecordingPositions{txIndexFilePath(baseDirectory)};
            for (size_t i = 0; i < txRecordingPositions.size(); i++) {
                timestampFiles.emplace_back(nthTxFilePath(baseDirectory, i), *txRecordingPositions[i]);
            }
            blockTimeFiles.clear();
            FixedSizeFileMapper<int> blockRecordingPositions{blockIndexFilePath(baseDirectory)};
            for (size_t i = 0; i < blockRecordingPositions.size(); i++) {
                blockTimeFiles.emplace_back(nthBlockFilePath(baseDirectory, i), *blockRecordingPositions[i]);
            }
        }
        
        static std::string txIndexFilePath(const std::string &baseDirectory);
        static std::string blockIndexFilePath(const std::string &baseDirectory);
        static std::string nthTxFilePath(const std::string &baseDirectory, size_t i);
        static std::string nthBlockFilePath(const std::string &baseDirectory, size_t i);
        
    public:
        explicit MempoolIndex(std::string baseDirectory_) :  baseDirectory(std::move(baseDirectory_)) {
            setup();
        }
        
        ranges::optional<std::reference_wrapper<const TimestampIndex>> selectPossibleTxRecording(uint32_t txIndex) const {
            if (timestampFiles.size() > 0) {
                auto it = ranges::upper_bound(timestampFiles, txIndex, ranges::ordered_less(), &TimestampIndex::firstTxIndex);
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
                auto it = ranges::upper_bound(blockTimeFiles, height, ranges::ordered_less(), &BlocktimeIndex::firstBlockNum);
                if (it != blockTimeFiles.begin()) {
                    --it;
                }
                if (it->contains(height)) {
                    return std::ref(*it);
                }
            }
            return ranges::nullopt;
        }

        ranges::optional<std::chrono::system_clock::time_point> getTxTimestamp(uint32_t index) const {
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
        
        ranges::optional<std::chrono::system_clock::time_point> getBlockTimestamp(int height) const {
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
