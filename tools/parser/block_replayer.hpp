//
//  block_replayer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/22/17.
//
//

#ifndef block_replayer_hpp
#define block_replayer_hpp

#include "config.hpp"
#include "parser_fwd.hpp"

#include <blocksci/typedefs.hpp>

#include <cstdint>

#ifdef BLOCKSCI_FILE_PARSER
void replayBlock(const ParserConfiguration<FileTag> &config, blocksci::BlockHeight blockNum);
#endif

#endif /* block_replayer_hpp */
