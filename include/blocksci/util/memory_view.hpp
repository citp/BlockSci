//
//  memory_view.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/14/18.
//

#ifndef memory_view_h
#define memory_view_h

#include <cstddef>

namespace blocksci {
    struct MemoryView {
        const char *data;
        size_t size;
    };
} // namespace blocksci

#endif /* memory_view_h */
