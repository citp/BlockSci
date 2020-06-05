//
//  exception.hpp
//  bitcoinapi
//
//  Created by Harry Kalodner on 4/15/18.
//

#ifndef blocksci_internal_exception_hpp
#define blocksci_internal_exception_hpp

#include <stdexcept>

namespace blocksci {
    class ReorgException : public std::runtime_error {
    public:
        ReorgException() : std::runtime_error("Blockchain has experienced reorg") {}
        ReorgException(const ReorgException &) = default;
        ReorgException(ReorgException &&) = default;
        ReorgException &operator=(const ReorgException &) = default;
        ReorgException &operator=(ReorgException &&) = default;
        virtual ~ReorgException();
    };
} // namespace blocksci

#endif /* blocksci_internal_exception_hpp */
