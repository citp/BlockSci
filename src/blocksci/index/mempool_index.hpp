//
//  mempool_index.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/12/18.
//

#ifndef mempool_index_hpp
#define mempool_index_hpp

#include <stdio.h>

#include <blocksci/blocksci_export.h>

#include <blocksci/util/file_mapper.hpp>
#include <blocksci/util/data_configuration.hpp>

namespace blocksci {
    
    class MempoolIndex {
        FixedSizeFileMapper<time_t> timestampFile;
        uint32_t beginTxIndex;
        uint32_t endTxIndex;
        
        void setup() {
            if (timestampFile.size() == 0) {
                // Mempool recorded has never been run
                beginTxIndex = 0;
                endTxIndex = 0;
            } else {
                // Fill in 0 timestamp where data is missing
                const auto &constView = timestampFile;
                beginTxIndex = *constView.getDataAtIndex(0);
                endTxIndex = beginTxIndex + timestampFile.size() - 1;
            }
        }
        
    public:
        explicit MempoolIndex(const DataConfiguration &config) : timestampFile(config.dataDirectory/"mempool") {
            setup();
        }

        ranges::optional<std::chrono::system_clock::time_point> getTimestamp(uint32_t index) const {
            if (index < beginTxIndex || index >= endTxIndex) {
                return ranges::nullopt;
            }
            auto *timestamp = timestampFile.getData(index - beginTxIndex + 1);
            if (*timestamp == 0) {
                return ranges::nullopt;
            }
            return std::chrono::system_clock::from_time_t(*timestamp);
        }
        
        void reload() {
            timestampFile.reload();
            setup();
        }
    };
} // namespace blocksci

#endif /* mempool_index_hpp */
