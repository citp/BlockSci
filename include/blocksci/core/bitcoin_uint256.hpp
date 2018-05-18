// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BLOCKSCI_BITCOIN_UINT256_H
#define BLOCKSCI_BITCOIN_UINT256_H

#include <blocksci/blocksci_export.h>

#include <cassert>
#include <cstring>
#include <functional>
#include <string>

namespace blocksci {
    
    template<typename T>
    std::string BLOCKSCI_EXPORT HexStr(const T itbegin, const T itend, bool fSpaces=false)
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
    inline std::string BLOCKSCI_EXPORT HexStr(const T& vch, bool fSpaces=false)
    {
        return HexStr(vch.begin(), vch.end(), fSpaces);
    }
    
    /** Template base class for fixed-sized opaque blobs. */
    template<unsigned int BITS>
    class base_blob
    {
    public:
        enum { WIDTH=BITS/8 };
        uint8_t data[WIDTH];

        base_blob()
        {
            memset(data, 0, sizeof(data));
        }
        
        template<typename It>
        base_blob(It begin, It end) {
            assert(std::distance(begin, end) == sizeof(data));
            std::copy(begin, end, data);
        }
        
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
        
        std::string GetHex() const {
            return HexStr(std::reverse_iterator<const uint8_t*>(data + sizeof(data)), std::reverse_iterator<const uint8_t*>(data));
        }
        
        std::string GetHexReverse() const {
            return HexStr(data, data + sizeof(data));
        }
        
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
    class BLOCKSCI_EXPORT uint160 : public base_blob<160> {
    public:
        uint160() {}
        explicit uint160(const base_blob<160>& b) : base_blob<160>(b) {}
        template<typename It> uint160(It begin, It end) : base_blob<160>(begin, end) {}
    };
    
    class BLOCKSCI_EXPORT uint256 : public base_blob<256> {
    public:
        uint256() {}
        explicit uint256(const base_blob<256>& b) : base_blob<256>(b) {}
        template<typename It> uint256(It begin, It end) : base_blob<256>(begin, end) {}
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

#endif // BLOCKSCI_BITCOIN_UINT256_H
