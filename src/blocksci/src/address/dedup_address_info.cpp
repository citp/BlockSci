//
//  dedup_address_info.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/7/18.
//

#include <blocksci/address/dedup_address_info.hpp>

#include <string>

namespace blocksci {
    constexpr char DedupAddressInfo<DedupAddressType::PUBKEY>::name[];
    constexpr char DedupAddressInfo<DedupAddressType::SCRIPTHASH>::name[];
    constexpr char DedupAddressInfo<DedupAddressType::MULTISIG>::name[];
    constexpr char DedupAddressInfo<DedupAddressType::NULL_DATA>::name[];
    constexpr char DedupAddressInfo<DedupAddressType::NONSTANDARD>::name[];
    
    constexpr std::array<AddressType::Enum, 4> DedupAddressInfo<DedupAddressType::PUBKEY>::addressTypes;
    constexpr std::array<AddressType::Enum, 2> DedupAddressInfo<DedupAddressType::SCRIPTHASH>::addressTypes;
    constexpr std::array<AddressType::Enum, 1> DedupAddressInfo<DedupAddressType::MULTISIG>::addressTypes;
    constexpr std::array<AddressType::Enum, 1> DedupAddressInfo<DedupAddressType::NULL_DATA>::addressTypes;
    constexpr std::array<AddressType::Enum, 1> DedupAddressInfo<DedupAddressType::NONSTANDARD>::addressTypes;
    
    template<DedupAddressType::Enum type>
    struct DedupAddressNameFunctor {
        static std::string f() {
            return DedupAddressInfo<type>::name;
        }
    };
    
    std::string dedupAddressName(DedupAddressType::Enum type) {
        static auto &table = *[]() {
            auto nameTable = make_static_table<DedupAddressType, DedupAddressNameFunctor>();
            return new decltype(nameTable){nameTable};
        }();
        auto index = static_cast<size_t>(type);
        return table.at(index);
    }
}
