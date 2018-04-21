//
//  hash.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/10/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <blocksci/util/hash.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>

#include <openssl/sha.h>
#include <openssl/ripemd.h>


blocksci::uint256 sha256(const uint8_t *data, size_t len) {
    blocksci::uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, len);
    SHA256_Final(reinterpret_cast<unsigned char *>(&hash), &sha256);
    return hash;
}

blocksci::uint256 sha256(const unsigned char *begin, const unsigned char *end) {
    return sha256(static_cast<const uint8_t *>(begin), static_cast<size_t>(end - begin));
}

bool base58_sha256(void *digest, const void *data, size_t datasz) {
    auto hash = sha256(reinterpret_cast<const uint8_t *>(data), datasz);
    memcpy(digest, &hash, sizeof(hash));
    return true;
}

blocksci::uint256 doubleSha256(const char *data, uint64_t len) {
    blocksci::uint256 txHash = sha256(reinterpret_cast<const uint8_t *>(data), len);
    txHash = sha256(reinterpret_cast<const uint8_t *>(&txHash), sizeof(blocksci::uint256));
    return txHash;
}

blocksci::uint160 ripemd160(const char *data, uint64_t len) {
    blocksci::uint160 hash;
    RIPEMD160_CTX ripemd;
    RIPEMD160_Init(&ripemd);
    RIPEMD160_Update(&ripemd, data, len);
    RIPEMD160_Final(reinterpret_cast<unsigned char *>(&hash), &ripemd);
    return hash;
}

blocksci::uint160 hash160(const void *data, uint64_t len) {
    auto hashed = sha256(reinterpret_cast<const uint8_t *>(data), len);
    return ripemd160(reinterpret_cast<const char *>(&hashed), sizeof(hashed));
}
