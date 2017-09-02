//
//  hash.cpp
//  blocksci_devel
//
//  Created by Harry Kalodner on 3/10/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "hash.hpp"

#include "crypto/sph_blake.h"
#include "crypto/sph_bmw.h"
#include "crypto/sph_groestl.h"
#include "crypto/sph_jh.h"
#include "crypto/sph_keccak.h"
#include "crypto/sph_skein.h"
#include "crypto/sph_luffa.h"
#include "crypto/sph_cubehash.h"
#include "crypto/sph_shavite.h"
#include "crypto/sph_simd.h"
#include "crypto/sph_echo.h"

#include <openssl/sha.h>
#include <openssl/ripemd.h>


blocksci::uint256 sha256(const uint8_t *data, size_t len) {
    blocksci::uint256 hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, len);
    SHA256_Final((unsigned char *)&hash, &sha256);
    return hash;
}

blocksci::uint256 sha256(const unsigned char *begin, const unsigned char *end) {
    return sha256(static_cast<const uint8_t *>(begin), end - begin);
}

bool base58_sha256(void *digest, const void *data, size_t datasz) {
    auto hash = sha256(reinterpret_cast<const uint8_t *>(data), datasz);
    memcpy(digest, &hash, sizeof(hash));
    return true;
}

blocksci::uint256 doubleSha256(const char *data, unsigned long len) {
    blocksci::uint256 txHash = sha256((const uint8_t *)data, len);
    txHash = sha256((const uint8_t *)&txHash, sizeof(blocksci::uint256));
    return txHash;
}

blocksci::uint160 ripemd160(const char *data, unsigned long len) {
    blocksci::uint160 hash;
    RIPEMD160_CTX ripemd;
    RIPEMD160_Init(&ripemd);
    RIPEMD160_Update(&ripemd, data, len);
    RIPEMD160_Final((unsigned char *)&hash, &ripemd);
    return hash;
}

blocksci::uint160 hash160(const void *data, unsigned long len) {
    auto hashed = sha256(reinterpret_cast<const uint8_t *>(data), len);
    return ripemd160(reinterpret_cast<const char *>(&hashed), sizeof(hashed));
}

/* ----------- Dash Hash ------------------------------------------------ */
blocksci::uint256 hashX11(const char *data, unsigned long len)

{
    sph_blake512_context     ctx_blake;
    sph_bmw512_context       ctx_bmw;
    sph_groestl512_context   ctx_groestl;
    sph_jh512_context        ctx_jh;
    sph_keccak512_context    ctx_keccak;
    sph_skein512_context     ctx_skein;
    sph_luffa512_context     ctx_luffa;
    sph_cubehash512_context  ctx_cubehash;
    sph_shavite512_context   ctx_shavite;
    sph_simd512_context      ctx_simd;
    sph_echo512_context      ctx_echo;
    
    blocksci::uint512 hash[11];
    
    sph_blake512_init(&ctx_blake);
    sph_blake512(&ctx_blake, data, len);
    sph_blake512_close(&ctx_blake, static_cast<void*>(&hash[0]));
    
    sph_bmw512_init(&ctx_bmw);
    sph_bmw512 (&ctx_bmw, static_cast<const void*>(&hash[0]), 64);
    sph_bmw512_close(&ctx_bmw, static_cast<void*>(&hash[1]));
    
    sph_groestl512_init(&ctx_groestl);
    sph_groestl512 (&ctx_groestl, static_cast<const void*>(&hash[1]), 64);
    sph_groestl512_close(&ctx_groestl, static_cast<void*>(&hash[2]));
    
    sph_skein512_init(&ctx_skein);
    sph_skein512 (&ctx_skein, static_cast<const void*>(&hash[2]), 64);
    sph_skein512_close(&ctx_skein, static_cast<void*>(&hash[3]));
    
    sph_jh512_init(&ctx_jh);
    sph_jh512 (&ctx_jh, static_cast<const void*>(&hash[3]), 64);
    sph_jh512_close(&ctx_jh, static_cast<void*>(&hash[4]));
    
    sph_keccak512_init(&ctx_keccak);
    sph_keccak512 (&ctx_keccak, static_cast<const void*>(&hash[4]), 64);
    sph_keccak512_close(&ctx_keccak, static_cast<void*>(&hash[5]));
    
    sph_luffa512_init(&ctx_luffa);
    sph_luffa512 (&ctx_luffa, static_cast<void*>(&hash[5]), 64);
    sph_luffa512_close(&ctx_luffa, static_cast<void*>(&hash[6]));
    
    sph_cubehash512_init(&ctx_cubehash);
    sph_cubehash512 (&ctx_cubehash, static_cast<const void*>(&hash[6]), 64);
    sph_cubehash512_close(&ctx_cubehash, static_cast<void*>(&hash[7]));
    
    sph_shavite512_init(&ctx_shavite);
    sph_shavite512(&ctx_shavite, static_cast<const void*>(&hash[7]), 64);
    sph_shavite512_close(&ctx_shavite, static_cast<void*>(&hash[8]));
    
    sph_simd512_init(&ctx_simd);
    sph_simd512 (&ctx_simd, static_cast<const void*>(&hash[8]), 64);
    sph_simd512_close(&ctx_simd, static_cast<void*>(&hash[9]));
    
    sph_echo512_init(&ctx_echo);
    sph_echo512 (&ctx_echo, static_cast<const void*>(&hash[9]), 64);
    sph_echo512_close(&ctx_echo, static_cast<void*>(&hash[10]));
    
    return hash[10].trim256();
}
