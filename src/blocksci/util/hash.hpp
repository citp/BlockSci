//
//  hash.hpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/10/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef hash_hpp
#define hash_hpp

#include "util/bitcoin_uint256.hpp"

template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

blocksci::uint256 sha256(const uint8_t *data, size_t len);
blocksci::uint256 sha256(const unsigned char *begin, const unsigned char *end);
blocksci::uint256 doubleSha256(const char *data, unsigned long len);
blocksci::uint160 ripemd160(const char *data, unsigned long len);
blocksci::uint160 hash160(const void *data, unsigned long len);

bool base58_sha256(void *digest, const void *data, size_t datasz);

#endif /* hash_hpp */
