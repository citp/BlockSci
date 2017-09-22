// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bitcoin_uint256.hpp"

#include <stdio.h>
#include <string.h>

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
    base_blob<BITS>::base_blob(const std::vector<unsigned char>& vch)
    {
        assert(vch.size() == sizeof(data));
        memcpy(data, &vch[0], sizeof(data));
    }
    
    template <unsigned int BITS>
    std::string base_blob<BITS>::GetHex() const
    {
        return HexStr(std::reverse_iterator<const uint8_t*>(data + sizeof(data)), std::reverse_iterator<const uint8_t*>(data));
    }
    
    template <unsigned int BITS>
    void base_blob<BITS>::SetHex(const char* psz)
    {
        memset(data, 0, sizeof(data));
        
        // skip leading spaces
        while (isspace(*psz))
            psz++;
        
        // skip 0x
        if (psz[0] == '0' && tolower(psz[1]) == 'x')
            psz += 2;
        
        // hex string to uint
        const char* pbegin = psz;
        while (HexDigit(*psz) != -1)
            psz++;
        psz--;
        unsigned char* p1 = reinterpret_cast<unsigned char *>(data);
        unsigned char* pend = p1 + WIDTH;
        while (psz >= pbegin && p1 < pend) {
            *p1 = static_cast<unsigned char>(HexDigit(*psz--));
            if (psz >= pbegin) {
                *p1 |= (static_cast<unsigned char>(HexDigit(*psz--)) << 4);
                p1++;
            }
        }
    }
    
    template <unsigned int BITS>
    void base_blob<BITS>::SetHex(const std::string& str)
    {
        SetHex(str.c_str());
    }
    
    template <unsigned int BITS>
    std::string base_blob<BITS>::ToString() const
    {
        return (GetHex());
    }
    
    uint160::uint160(const std::vector<unsigned char>& vch) : base_blob<160>(vch) {}
    uint256::uint256(const std::vector<unsigned char>& vch) : base_blob<256>(vch) {}
    
    // Explicit instantiations for base_blob<160>
    template base_blob<160>::base_blob(const std::vector<unsigned char>&);
    template std::string base_blob<160>::GetHex() const;
    template std::string base_blob<160>::ToString() const;
    template void base_blob<160>::SetHex(const char*);
    template void base_blob<160>::SetHex(const std::string&);
    
    // Explicit instantiations for base_blob<256>
    template base_blob<256>::base_blob(const std::vector<unsigned char>&);
    template std::string base_blob<256>::GetHex() const;
    template std::string base_blob<256>::ToString() const;
    template void base_blob<256>::SetHex(const char*);
    template void base_blob<256>::SetHex(const std::string&);
    
    uint160 uint160S(const char *str) {
        uint160 rv;
        rv.SetHex(str);
        return rv;
    }
    
    uint160 uint160S(const std::string& str) {
        uint160 rv;
        rv.SetHex(str);
        return rv;
    }
    
    uint256 uint256S(const char *str) {
        uint256 rv;
        rv.SetHex(str);
        return rv;
    }
    
    uint256 uint256S(const std::string& str)
    {
        uint256 rv;
        rv.SetHex(str);
        return rv;
    }

}

