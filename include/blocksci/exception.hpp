//
//  exception.hpp
//  bitcoinapi
//
//  Created by Harry Kalodner on 4/15/18.
//

#ifndef blocksci_exception_hpp
#define blocksci_exception_hpp

#include <blocksci/blocksci_export.h>
#include <stdexcept>

namespace blocksci {
    class BLOCKSCI_EXPORT InvalidAddressException : virtual public std::runtime_error {
    public:
        InvalidAddressException() : std::runtime_error("Tried to construct invalid address") {}
        InvalidAddressException(const InvalidAddressException &) = default;
        InvalidAddressException(InvalidAddressException &&) = default;
        InvalidAddressException &operator=(const InvalidAddressException &) = default;
        InvalidAddressException &operator=(InvalidAddressException &&) = default;
        virtual ~InvalidAddressException();
    };
} // namespace blocksci

#endif /* blocksci_exception_hpp */
