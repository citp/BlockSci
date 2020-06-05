//
//  nonstandard_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef nonstandard_script_hpp
#define nonstandard_script_hpp

#include "script.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/core/hash_combine.hpp>

#include <range/v3/utility/optional.hpp>

#include <string>

namespace blocksci {
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::NONSTANDARD> : public ScriptBase {
        const NonstandardSpendScriptData *rawInputData;
        
        const NonstandardScriptData *getData() const {
            return reinterpret_cast<const NonstandardScriptData *>(ScriptBase::getData());
        }
        
        ranges::optional<CScriptView> getInputScript() const;
        
        CScriptView getOutputScript() const;
        
    public:
        constexpr static AddressType::Enum addressType = AddressType::NONSTANDARD;
        
        ScriptAddress() = default;
        ScriptAddress(uint32_t scriptNum_, std::tuple<const NonstandardScriptData *, const NonstandardSpendScriptData *> &&rawData_, DataAccess &access_) : ScriptBase(scriptNum_, addressType, access_, std::get<0>(rawData_)), rawInputData(std::get<1>(rawData_)) {}
        ScriptAddress(uint32_t addressNum_, DataAccess &access_);
        
        std::string inputString() const;
        std::string outputString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
    };
} // namespace blocksci

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::ScriptAddress<blocksci::AddressType::NONSTANDARD>> {
        size_t operator()(const blocksci::ScriptAddress<blocksci::AddressType::NONSTANDARD> &address) const {
            std::size_t seed = 1357869;
            blocksci::hash_combine(seed, static_cast<const blocksci::ScriptBase &>(address));
            return seed;
        }
    };
} // namespace std

#endif /* nonstandard_script_hpp */
