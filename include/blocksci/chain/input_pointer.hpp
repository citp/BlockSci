//
//  input_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef input_pointer_hpp
#define input_pointer_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/core/inout_pointer.hpp>

#include <string>

namespace blocksci {

    /** Contains data to uniquely identify one input using txNum and inoutNum */
    struct BLOCKSCI_EXPORT InputPointer : public InoutPointer {
        using InoutPointer::InoutPointer;
                
        std::string toString() const;
    };
} // namespace blocksci

std::ostream BLOCKSCI_EXPORT &operator<<(std::ostream &os, const blocksci::InputPointer &pointer);

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::InputPointer> {
        size_t operator()(const blocksci::InputPointer &pointer) const {
            std::size_t seed = 754634;
            blocksci::hash_combine(seed, static_cast<const blocksci::InoutPointer &>(pointer));
            return seed;
        }
    };
} // namespace std

#endif /* input_pointer_hpp */
