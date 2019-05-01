// Copyright (c) 2014-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bitcoin_base58.hpp"

#include <blocksci/core/bitcoin_uint256.hpp>

#include <internal/hash.hpp>
#include <internal/data_configuration.hpp>

#include <cassert>
#include <string>
#include <vector>

namespace blocksci {

    /** All alphanumeric characters except for "0", "I", "O", and "l" */
    static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

    bool DecodeBase58(const char* psz, std::vector<unsigned char>& vch)
    {
        // Skip leading spaces.
        while (*psz && isspace(*psz))
            psz++;
        // Skip and count leading '1's.
        size_t zeroes = 0;
        size_t length = 0;
        while (*psz == '1') {
            zeroes++;
            psz++;
        }
        // Allocate enough space in big-endian base256 representation.
        size_t size = strlen(psz) * 733 /1000 + 1; // log(58) / log(256), rounded up.
        std::vector<unsigned char> b256(size);
        // Process the characters.
        while (*psz && !isspace(*psz)) {
            // Decode base58 character
            const char* ch = strchr(pszBase58, *psz);
            if (ch == nullptr)
                return false;
            // Apply "b256 = b256 * 58 + ch".
            auto carry = ch - pszBase58;
            size_t i = 0;
            for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
                carry += 58 * (*it);
                *it = static_cast<unsigned char>(carry % 256);
                carry /= 256;
            }
            assert(carry == 0);
            length = i;
            psz++;
        }
        // Skip trailing spaces.
        while (isspace(*psz))
            psz++;
        if (*psz != 0)
            return false;
        // Skip leading zeroes in b256.
        auto it = b256.begin();
        std::advance(it, static_cast<int64_t>(size - length));
        while (it != b256.end() && *it == 0)
            ++it;
        // Copy result into output vector.
        vch.reserve(zeroes + static_cast<size_t>(std::distance(it, b256.end())));
        vch.assign(zeroes, 0x00);
        while (it != b256.end())
            vch.push_back(*(it++));
        return true;
    }

    std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend)
    {
        // Skip & count leading zeroes.
        size_t zeroes = 0;
        size_t length = 0;
        while (pbegin != pend && *pbegin == 0) {
            pbegin++;
            zeroes++;
        }
        // Allocate enough space in big-endian base58 representation.
        size_t size = static_cast<size_t>((pend - pbegin) * 138 / 100 + 1); // log(256) / log(58), rounded up.
        std::vector<unsigned char> b58(size);
        // Process the bytes.
        while (pbegin != pend) {
            int carry = *pbegin;
            size_t i = 0;
            // Apply "b58 = b58 * 256 + ch".
            for (auto it = b58.rbegin(); (carry != 0 || i < length) && (it != b58.rend()); ++it, ++i) {
                carry += 256 * (*it);
                *it = carry % 58;
                carry /= 58;
            }

            assert(carry == 0);
            length = i;
            pbegin++;
        }
        // Skip leading zeroes in base58 result.
        auto it = b58.begin();
        std::advance(it, static_cast<int64_t>(size - length));
        while (it != b58.end() && *it == 0)
            ++it;
        // Translate the result into a string.
        std::string str;
        str.reserve(zeroes + static_cast<size_t>(std::distance(it, b58.end())));
        str.assign(zeroes, '1');
        while (it != b58.end())
            str += pszBase58[*(it++)];
        return str;
    }

    std::string EncodeBase58(const std::vector<unsigned char>& vch)
    {
        return EncodeBase58(vch.data(), vch.data() + vch.size());
    }

    bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet)
    {
        return DecodeBase58(str.c_str(), vchRet);
    }

    std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn)
    {
        // add 4-byte hash check to the end
        std::vector<unsigned char> vch(vchIn);
        uint256 hash = doubleSha256(reinterpret_cast<const char*>(vch.data()), vch.size());
        vch.insert(vch.end(), reinterpret_cast<unsigned char*>(&hash), reinterpret_cast<unsigned char*>(&hash) + 4);
        return EncodeBase58(vch);
    }

    bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet)
    {
        if (!DecodeBase58(psz, vchRet) ||
            (vchRet.size() < 4)) {
            vchRet.clear();
            return false;
        }
        // Ignore the checksum since it shouldn't matter for our purposes
        vchRet.resize(vchRet.size() - 4);
        return true;
    }

    bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet)
    {
        return DecodeBase58Check(str.c_str(), vchRet);
    }

    CBase58Data::CBase58Data()
    {
        vchVersion.clear();
        vchData.clear();
    }

    void CBase58Data::SetData(const std::vector<unsigned char>& vchVersionIn, const void* pdata, size_t nSize)
    {
        vchVersion = vchVersionIn;
        vchData.resize(nSize);
        if (!vchData.empty())
            memcpy(vchData.data(), pdata, nSize);
    }

    void CBase58Data::SetData(const std::vector<unsigned char>& vchVersionIn, const unsigned char* pbegin, const unsigned char* pend)
    {
        SetData(vchVersionIn, reinterpret_cast<const void *>(pbegin), static_cast<size_t>(pend - pbegin));
    }

    bool CBase58Data::SetString(const char* psz, unsigned int nVersionBytes)
    {
        std::vector<unsigned char> vchTemp;
        bool rc58 = DecodeBase58Check(psz, vchTemp);
        if ((!rc58) || (vchTemp.size() < nVersionBytes)) {
            vchData.clear();
            vchVersion.clear();
            throw InvalidAddressException();
        }
        vchVersion.assign(vchTemp.begin(), vchTemp.begin() + nVersionBytes);
        vchData.resize(vchTemp.size() - nVersionBytes);
        if (!vchData.empty())
            memcpy(vchData.data(), vchTemp.data() + nVersionBytes, vchData.size());
        return true;
    }

    bool CBase58Data::SetString(const std::string& str, unsigned int nVersionBytes)
    {
        return SetString(str.c_str(), nVersionBytes);
    }

    std::string CBase58Data::ToString() const
    {
        std::vector<unsigned char> vch = vchVersion;
        vch.insert(vch.end(), vchData.begin(), vchData.end());
        return EncodeBase58Check(vch);
    }

    int CBase58Data::CompareTo(const CBase58Data& b58) const
    {
        if (vchVersion < b58.vchVersion)
            return -1;
        if (vchVersion > b58.vchVersion)
            return 1;
        if (vchData < b58.vchData)
            return -1;
        if (vchData > b58.vchData)
            return 1;
        return 0;
    }


    CBitcoinAddress::CBitcoinAddress(const uint160 &dest, AddressType::Enum type, const ChainConfiguration &config) {
        if (type == AddressType::Enum::PUBKEYHASH || type == AddressType::Enum::PUBKEY || type == AddressType::Enum::MULTISIG_PUBKEY) {
            SetData(config.pubkeyPrefix, &dest, sizeof(dest));
        } else if (type == AddressType::Enum::SCRIPTHASH) {
            SetData(config.scriptPrefix, &dest, sizeof(dest));
        }
    }
    
    CBitcoinAddress::CBitcoinAddress(const uint160 &dest, const std::vector<unsigned char>& version) {
        SetData(version, &dest, sizeof(dest));
    }

    std::pair<uint160, AddressType::Enum> CBitcoinAddress::Get(const ChainConfiguration &config) const {
        uint160 id;
        memcpy(&id, vchData.data(), sizeof(id));
        if (vchVersion == config.pubkeyPrefix)
            return std::make_pair(id, AddressType::Enum::PUBKEYHASH);
        else if (vchVersion == config.scriptPrefix)
            return std::make_pair(id, AddressType::Enum::SCRIPTHASH);
        else
            return std::make_pair(id, AddressType::Enum::NONSTANDARD);
    }
} // namespace blocksci
