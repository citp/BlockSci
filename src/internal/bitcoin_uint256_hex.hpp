// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BLOCKSCI_BITCOIN_UINT256_HEX_H
#define BLOCKSCI_BITCOIN_UINT256_HEX_H

#include <blocksci/core/bitcoin_uint256.hpp>

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>

namespace blocksci {
    
    signed char HexDigit(char c);
    
    /* uint160 from const char *.
     * This is a separate function because the constructor uint256(const char*) can result
     * in dangerously catching uint160(0).
     */
    uint160 uint160S(const char *str);
    /* uint160 from std::string.
     * This is a separate function because the constructor uint256(const std::string &str) can result
     * in dangerously catching uint160(0) via std::string(const char*).
     */
    uint160 uint160S(const std::string& str);
    
    /* uint256 from const char *.
     * This is a separate function because the constructor uint256(const char*) can result
     * in dangerously catching uint256(0).
     */
    uint256 uint256S(const char *str);
    
    /* uint256 from std::string.
     * This is a separate function because the constructor uint256(const std::string &str) can result
     * in dangerously catching uint256(0) via std::string(const char*).
     */
    uint256 uint256S(const std::string& str);
}

#endif // BLOCKSCI_BITCOIN_UINT256_HEX_H
