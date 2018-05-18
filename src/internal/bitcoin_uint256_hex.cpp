// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bitcoin_uint256_hex.hpp"

#include <string>

namespace blocksci {
    
    signed char HexDigit(char c);
    
    const signed char p_util_hexdigit[256] =
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,
        -1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, };
    
    signed char HexDigit(char c)
    {
        return p_util_hexdigit[static_cast<unsigned char>(c)];
    }
    
    template <unsigned int BITS>
    base_blob<BITS> hexToBlob(const char* psz)
    {
        base_blob<BITS> blob;
        blob.SetNull();
        
        // skip leading spaces
        while (isspace(*psz)) {
            psz++;
        }
        
        // skip 0x
        if (psz[0] == '0' && tolower(psz[1]) == 'x') {
            psz += 2;
        }
        
        // hex string to uint
        const char* pbegin = psz;
        while (HexDigit(*psz) != -1) {
            psz++;
        }
        psz--;
        auto p1 = blob.begin();
        unsigned char* pend = blob.end();
        while (psz >= pbegin && p1 < pend) {
            *p1 = static_cast<unsigned char>(HexDigit(*psz--));
            if (psz >= pbegin) {
                *p1 |= (static_cast<unsigned char>(HexDigit(*psz--)) << 4);
                p1++;
            }
        }
        return blob;
    }

    uint160 uint160S(const char *str) {
        return uint160{hexToBlob<160>(str)};
    }
    
    uint160 uint160S(const std::string& str) {
        return uint160{hexToBlob<160>(str.c_str())};
    }
    
    uint256 uint256S(const char *str) {
        return uint256{hexToBlob<256>(str)};
    }
    
    uint256 uint256S(const std::string& str) {
        return uint256{hexToBlob<256>(str.c_str())};
    }

}

