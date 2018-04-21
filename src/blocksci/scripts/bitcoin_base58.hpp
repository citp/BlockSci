// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Why base-58 instead of standard base-64 encoding?
 * - Don't want 0OIl characters that look the same in some fonts and
 *      could be used to create visually identical looking data.
 * - A string with non-alphanumeric characters is not as easily accepted as input.
 * - E-mail usually won't line-break if there's no punctuation to break at.
 * - Double-clicking selects the whole string as one word if it's all alphanumeric.
 */
#ifndef BLOCKSCI_BITCOIN_BASE58_H
#define BLOCKSCI_BITCOIN_BASE58_H

#include <blocksci/exception.hpp>
#include <blocksci/address/address_types.hpp>

#include <string>
#include <vector>

namespace blocksci {
    
    struct DataConfiguration;
    class uint160;
    
    /**
     * Encode a byte sequence as a base58-encoded string.
     * pbegin and pend cannot be nullptr, unless both are.
     */
    std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend);
    
    /**
     * Encode a byte vector as a base58-encoded string
     */
    std::string EncodeBase58(const std::vector<unsigned char>& vch);
    
    /**
     * Decode a base58-encoded string (psz) into a byte vector (vchRet).
     * return true if decoding is successful.
     * psz cannot be nullptr.
     */
    bool DecodeBase58(const char* psz, std::vector<unsigned char>& vchRet);
    
    /**
     * Decode a base58-encoded string (str) into a byte vector (vchRet).
     * return true if decoding is successful.
     */
    bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet);
    
    /**
     * Encode a byte vector into a base58-encoded string, including checksum
     */
    std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn);
    
    /**
     * Decode a base58-encoded string (psz) that includes a checksum into a byte
     * vector (vchRet), return true if decoding is successful
     */
    inline bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet);
    
    /**
     * Decode a base58-encoded string (str) that includes a checksum into a byte
     * vector (vchRet), return true if decoding is successful
     */
    inline bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet);
    
    /**
     * Base class for all base58-encoded data
     */
    class CBase58Data
    {
    protected:
        //! the version byte(s)
        std::vector<unsigned char> vchVersion;
        
        //! the actually encoded data
        typedef std::vector<unsigned char> vector_uchar;
        vector_uchar vchData;
        
        CBase58Data();
        void SetData(const std::vector<unsigned char> &vchVersionIn, const void* pdata, size_t nSize);
        void SetData(const std::vector<unsigned char> &vchVersionIn, const unsigned char *pbegin, const unsigned char *pend);
        
    public:
        bool SetString(const char* psz, unsigned int nVersionBytes = 1);
        bool SetString(const std::string& str);
        std::string ToString() const;
        int CompareTo(const CBase58Data& b58) const;
        
        bool operator==(const CBase58Data& b58) const { return CompareTo(b58) == 0; }
        bool operator<=(const CBase58Data& b58) const { return CompareTo(b58) <= 0; }
        bool operator>=(const CBase58Data& b58) const { return CompareTo(b58) >= 0; }
        bool operator< (const CBase58Data& b58) const { return CompareTo(b58) <  0; }
        bool operator> (const CBase58Data& b58) const { return CompareTo(b58) >  0; }
    };
    
    /** base58-encoded Bitcoin addresses.
     * Public-key-hash-addresses have version 0 (or 111 testnet).
     * The data vector contains RIPEMD160(SHA256(pubkey)), where pubkey is the serialized public key.
     * Script-hash-addresses have version 5 (or 196 testnet).
     * The data vector contains RIPEMD160(SHA256(cscript)), where cscript is the serialized redemption script.
     */
    class CBitcoinAddress : public CBase58Data {
    public:
        CBitcoinAddress(const uint160 &dest, AddressType::Enum type, const DataConfiguration &config);
        CBitcoinAddress(const uint160 &dest, const std::vector<unsigned char>& version);
        CBitcoinAddress(const std::string& strAddress) { SetString(strAddress); }
        CBitcoinAddress(const char* pszAddress) { SetString(pszAddress); }
        
        std::pair<uint160, AddressType::Enum> Get(const DataConfiguration &config) const;
    };
} // namespace blocksci

#endif // BLOCKSCI_BITCOIN_BASE58_H
