/* Copyright (c) 2017 Pieter Wuille
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "bitcoin_segwit_addr.h"
#include "bitcoin_bech32.h"
#include <blocksci/util/data_configuration.hpp>

namespace
{

typedef std::vector<uint8_t> data;

/** Convert from one power-of-2 number base to another. */
template<int frombits, int tobits, bool pad>
bool convertbits(data& out, const data& in) {
    int acc = 0;
    int bits = 0;
    const int maxv = (1 << tobits) - 1;
    const int max_acc = (1 << (frombits + tobits - 1)) - 1;
    for (size_t i = 0; i < in.size(); ++i) {
        int value = in[i];
        acc = ((acc << frombits) | value) & max_acc;
        bits += frombits;
        while (bits >= tobits) {
            bits -= tobits;
            out.push_back((acc >> bits) & maxv);
        }
    }
    if (pad) {
        if (bits) out.push_back((acc << (tobits - bits)) & maxv);
    } else if (bits >= frombits || ((acc << (tobits - bits)) & maxv)) {
        return false;
    }
    return true;
}

}

namespace segwit_addr
{

/** Decode a SegWit address. */
std::pair<int, data> decode(const std::string& hrp, const std::string& addr) {
    std::pair<std::string, data> dec = bech32::decode(addr);
    if (dec.first != hrp || dec.second.size() < 1) return std::make_pair(-1, data());
    data conv;
    if (!convertbits<5, 8, false>(conv, data(dec.second.begin() + 1, dec.second.end())) ||
        conv.size() < 2 || conv.size() > 40 || dec.second[0] > 16 || (dec.second[0] == 0 &&
        conv.size() != 20 && conv.size() != 32)) {
        return std::make_pair(-1, data());
    }
    return std::make_pair(dec.second[0], conv);
}

/** Encode a SegWit address. */
std::string encode(const std::string& hrp, int witver, const data& witprog) {
    data enc;
    enc.push_back(witver);
    convertbits<8, 5, true>(enc, witprog);
    std::string ret = bech32::encode(hrp, enc);
    if (decode(hrp, ret).first == -1) return "";
    return ret;
}
    
std::string encode(const blocksci::DataConfiguration &config, int witver, const data& witprog) {
    data enc;
    enc.push_back(witver);
    convertbits<8, 5, true>(enc, witprog);
    std::string ret = bech32::encode(config.segwitPrefix, enc);
    if (decode(config.segwitPrefix, ret).first == -1) return "";
    return ret;
}

}
