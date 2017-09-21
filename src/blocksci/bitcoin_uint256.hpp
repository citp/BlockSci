// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_UINT256_H
#define BITCOIN_UINT256_H

#include <assert.h>
#include <cstring>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>

namespace blocksci {
    
    signed char HexDigit(char c);
    
    template<typename T>
    std::string HexStr(const T itbegin, const T itend, bool fSpaces=false)
    {
        std::string rv;
        static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
        rv.reserve(static_cast<size_t>(itend-itbegin)*3);
        for(T it = itbegin; it < itend; ++it)
        {
            unsigned char val = static_cast<unsigned char>(*it);
            if(fSpaces && it != itbegin)
                rv.push_back(' ');
            rv.push_back(hexmap[val>>4]);
            rv.push_back(hexmap[val&15]);
        }
        
        return rv;
    }
    
    template<typename T>
    inline std::string HexStr(const T& vch, bool fSpaces=false)
    {
        return HexStr(vch.begin(), vch.end(), fSpaces);
    }
    
    /** Template base class for fixed-sized opaque blobs. */
    template<unsigned int BITS>
    class base_blob
    {
    protected:
        enum { WIDTH=BITS/8 };
        uint8_t data[WIDTH];
    public:
        base_blob()
        {
            memset(data, 0, sizeof(data));
        }
        
        explicit base_blob(const std::vector<unsigned char>& vch);
        
        bool IsNull() const
        {
            for (int i = 0; i < WIDTH; i++)
                if (data[i] != 0)
                    return false;
            return true;
        }
        
        void SetNull()
        {
            memset(data, 0, sizeof(data));
        }
        
        inline int Compare(const base_blob& other) const { return memcmp(data, other.data, sizeof(data)); }
        
        friend inline bool operator==(const base_blob& a, const base_blob& b) { return a.Compare(b) == 0; }
        friend inline bool operator!=(const base_blob& a, const base_blob& b) { return a.Compare(b) != 0; }
        friend inline bool operator<(const base_blob& a, const base_blob& b) { return a.Compare(b) < 0; }
        
        std::string GetHex() const;
        void SetHex(const char* psz);
        void SetHex(const std::string& str);
        std::string ToString() const;
        
        unsigned char* begin()
        {
            return &data[0];
        }
        
        unsigned char* end()
        {
            return &data[WIDTH];
        }
        
        const unsigned char* begin() const
        {
            return &data[0];
        }
        
        const unsigned char* end() const
        {
            return &data[WIDTH];
        }
        
        unsigned int size() const
        {
            return sizeof(data);
        }
        
        uint64_t GetUint64(int pos) const
        {
            const uint8_t* ptr = data + pos * 8;
            return (static_cast<uint64_t>(ptr[0])) | \
            (static_cast<uint64_t>(ptr[1])) << 8 | \
            (static_cast<uint64_t>(ptr[2])) << 16 | \
            (static_cast<uint64_t>(ptr[3])) << 24 | \
            (static_cast<uint64_t>(ptr[4])) << 32 | \
            (static_cast<uint64_t>(ptr[5])) << 40 | \
            (static_cast<uint64_t>(ptr[6])) << 48 | \
            (static_cast<uint64_t>(ptr[7])) << 56;
        }
        
        template<typename Stream>
        void Serialize(Stream& s) const
        {
            s.write(reinterpret_cast<char *>(data), sizeof(data));
        }
        
        template<typename Stream>
        void Unserialize(Stream& s)
        {
            s.read(reinterpret_cast<char *>(data), sizeof(data));
        }
    };
    
    /** 160-bit opaque blob.
     * @note This type is called uint160 for historical reasons only. It is an opaque
     * blob of 160 bits and has no integer operations.
     */
    class uint160 : public base_blob<160> {
    public:
        uint160() {}
        uint160(const base_blob<160>& b) : base_blob<160>(b) {}
        explicit uint160(const std::vector<unsigned char>& vch) : base_blob<160>(vch) {}
    };
    
    /* uint256 from const char *.
     * This is a separate function because the constructor uint256(const char*) can result
     * in dangerously catching uint256(0).
     */
    inline uint160 uint160S(const char *str)
    {
        uint160 rv;
        rv.SetHex(str);
        return rv;
    }
    /* uint256 from std::string.
     * This is a separate function because the constructor uint256(const std::string &str) can result
     * in dangerously catching uint256(0) via std::string(const char*).
     */
    inline uint160 uint160S(const std::string& str)
    {
        uint160 rv;
        rv.SetHex(str);
        return rv;
    }
    
    /** 256-bit opaque blob.
     * @note This type is called uint256 for historical reasons only. It is an
     * opaque blob of 256 bits and has no integer operations. Use arith_uint256 if
     * those are required.
     */
    class uint256 : public base_blob<256> {
    public:
        uint256() {}
        uint256(const base_blob<256>& b) : base_blob<256>(b) {}
        explicit uint256(const std::vector<unsigned char>& vch) : base_blob<256>(vch) {}
    };
    
    /* uint256 from const char *.
     * This is a separate function because the constructor uint256(const char*) can result
     * in dangerously catching uint256(0).
     */
    inline uint256 uint256S(const char *str)
    {
        uint256 rv;
        rv.SetHex(str);
        return rv;
    }
    /* uint256 from std::string.
     * This is a separate function because the constructor uint256(const std::string &str) can result
     * in dangerously catching uint256(0) via std::string(const char*).
     */
    inline uint256 uint256S(const std::string& str)
    {
        uint256 rv;
        rv.SetHex(str);
        return rv;
    }
    
    /** 512-bit unsigned big integer. */
    class uint512 : public base_blob<512> {
    public:
        uint512() {}
        uint512(const base_blob<512>& b) : base_blob<512>(b) {}
        explicit uint512(const std::vector<unsigned char>& vch) : base_blob<512>(vch) {}
        
        uint256 trim256() const
        {
            uint256 result;
            memcpy(reinterpret_cast<void *>(&result), reinterpret_cast<const void *>(data), 32);
            return result;
        }
    };
}

namespace std {
    template <>
    struct hash<blocksci::uint256> {
    public:
        size_t operator()(const blocksci::uint256 &b) const {
            return b.GetUint64(2);
        }
    };
    template <>
    struct hash<blocksci::uint160> {
    public:
        size_t operator()(const blocksci::uint160 &b) const {
            return b.GetUint64(1);
        }
    };
}

#endif // BITCOIN_UINT256_H
