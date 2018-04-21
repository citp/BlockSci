//
//  file_mapper_fwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/17/17.
//

#ifndef file_mapper_fwd_h
#define file_mapper_fwd_h

namespace blocksci {
    enum class AccessMode {
        readonly, readwrite
    };
    
    template<AccessMode mode = AccessMode::readonly>
    struct SimpleFileMapper;
    
    template <typename T, AccessMode mode = AccessMode::readonly>
    struct FixedSizeFileMapper;
    
    template <AccessMode mode = AccessMode::readonly, typename... T>
    struct IndexedFileMapper;
}

#endif /* file_mapper_fwd_h */
