// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BLOCKSCI_BITCOIN_PUBKEY_H
#define BLOCKSCI_BITCOIN_PUBKEY_H

#include <blocksci/blocksci_export.h>
#include <blocksci/core/bitcoin_uint256.hpp>

#include <array>
#include <stdexcept>
#include <vector>


namespace blocksci {
    /**
     * secp256k1:
     *
     *
     * see www.keylength.com
     * script supports up to 75 for single byte push
     */
    
    const unsigned int BIP32_EXTKEY_SIZE = 74;
    
    /** A reference to a CKey: the Hash160 of its serialized public key */
    class BLOCKSCI_EXPORT CKeyID : public uint160
    {
    public:
        CKeyID() : uint160() {}
        explicit CKeyID(const uint160& in) : uint160(in) {}
    };
    
    typedef uint256 ChainCode;
    
    /** An encapsulated public key. */
    class BLOCKSCI_EXPORT CPubKey
    {
    public:
        static constexpr unsigned int PUBLIC_KEY_SIZE             = 65;
        static constexpr unsigned int COMPRESSED_PUBLIC_KEY_SIZE  = 33;
        static constexpr unsigned int SIGNATURE_SIZE              = 72;
        static constexpr unsigned int COMPACT_SIGNATURE_SIZE      = 65;
    private:
        
        /**
         * Just store the serialized data.
         * Its length can very cheaply be computed from the first byte.
         */
        std::array<unsigned char, 65> vch;
        
        //! Compute the length of a pubkey with a given first byte.
        unsigned int static GetLen(unsigned char chHeader)
        {
            if (chHeader == 2 || chHeader == 3)
                return 33;
            if (chHeader == 4 || chHeader == 6 || chHeader == 7)
                return 65;
            return 0;
        }
        
        //! Set this key data to be invalid
        void Invalidate()
        {
            vch[0] = 0xFF;
        }
        
    public:
        bool static ValidSize(const std::vector<unsigned char> &vch) {
            return vch.size() > 0 && GetLen(vch[0]) == vch.size();
        }
        
        //! Construct an invalid public key.
        CPubKey() : vch{{0}}
        {
            Invalidate();
        }
        
        //! Initialize a public key using begin/end iterators to byte data.
        template <typename T>
        void Set(const T pbegin, const T pend)
        {
            unsigned int len = pend == pbegin ? 0 : GetLen(pbegin[0]);
            if (len && len <= (pend - pbegin))
                memcpy(vch.data(), reinterpret_cast<const unsigned char*>(&pbegin[0]), len);
            else
                Invalidate();
        }
        
        //! Construct a public key using begin/end iterators to byte data.
        template <typename T>
        CPubKey(const T pbegin, const T pend)
        {
            Set(pbegin, pend);
        }
        
        //! Construct a public key from a byte vector.
        explicit CPubKey(const std::vector<unsigned char>& _vch)
        {
            Set(_vch.begin(), _vch.end());
        }
        
        //! Simple read-only vector-like interface to the pubkey data.
        unsigned int size() const { return GetLen(vch[0]); }
        const unsigned char* begin() const { return vch.data(); }
        const unsigned char* end() const { return vch.data() + size(); }
        const unsigned char& operator[](unsigned int pos) const { return vch[pos]; }
        
        //! Comparator implementation.
        friend bool operator==(const CPubKey& a, const CPubKey& b)
        {
            return a.vch[0] == b.vch[0] &&
            memcmp(a.vch.data(), b.vch.data(), a.size()) == 0;
        }
        friend bool operator!=(const CPubKey& a, const CPubKey& b)
        {
            return !(a == b);
        }
        friend bool operator<(const CPubKey& a, const CPubKey& b)
        {
            return a.vch[0] < b.vch[0] ||
            (a.vch[0] == b.vch[0] && memcmp(a.vch.data(), b.vch.data(), a.size()) < 0);
        }
        
        //! Get the KeyID of this public key (hash of its serialization)
        CKeyID GetID() const;
        
        //! Get the 256-bit hash of this public key.
        uint256 GetHash() const;
        
        /*
         * Check syntactic correctness.
         *
         * Note that this is consensus critical as CheckSig() calls it!
         */
        bool IsValid() const
        {
            return size() > 0;
        }
        
        //! fully validate whether this is a valid public key (more expensive than IsValid())
        bool IsFullyValid() const;
        
        //! Check whether this is a compressed public key.
        bool IsCompressed() const
        {
            return size() == 33;
        }
        
        /**
         * Verify a DER signature (~72 bytes).
         * If this public key is not fully valid, the return value will be false.
         */
        bool Verify(const uint256& hash, const std::vector<unsigned char>& vchSig) const;
        
        /**
         * Check whether a signature is normalized (lower-S).
         */
        static bool CheckLowS(const std::vector<unsigned char>& vchSig);
        
        //! Recover a public key from a compact signature.
        bool RecoverCompact(const uint256& hash, const std::vector<unsigned char>& vchSig);
        
        //! Turn this public key into an uncompressed public key.
        bool Decompress();
    };
    
    
    /** Users of this module must hold an ECCVerifyHandle. The constructor and
     *  destructor of these are not allowed to run in parallel, though. */
    class BLOCKSCI_EXPORT ECCVerifyHandle
    {
        static int refcount;
        
    public:
        ECCVerifyHandle();
        ~ECCVerifyHandle();
    };
} // namespace blocksci

#endif // BLOCKSCI_BITCOIN_PUBKEY_H
