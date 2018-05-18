//
//  hash.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/10/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef hash_hpp
#define hash_hpp

#include <cstdint>
#include <cstddef>

namespace blocksci {
    class uint256;
    class uint160;
}

blocksci::uint256 sha256(const uint8_t *data, size_t len);
blocksci::uint256 sha256(const unsigned char *begin, const unsigned char *end);
blocksci::uint256 doubleSha256(const char *data, uint64_t len);
blocksci::uint160 ripemd160(const char *data, uint64_t len);
blocksci::uint160 hash160(const void *data, uint64_t len);

bool base58_sha256(void *digest, const void *data, size_t datasz);

#endif /* hash_hpp */
