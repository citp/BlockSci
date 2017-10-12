//
//  utxo_state.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/10/17.
//
//

#ifndef utxo_state_hpp
#define utxo_state_hpp

#include "basic_types.hpp"
#include "utxo.hpp"
#include "parser_fwd.hpp"

#include <sparsepp/spp.h>

#include <boost/functional/hash.hpp>

#include <google/dense_hash_map>

#include <sstream>
#include <future>

class UTXOState {
    using utxo_map = google::dense_hash_map<RawOutputPointer, UTXO, boost::hash<RawOutputPointer>>;
    
    const ParserConfigurationBase &config;
    
    utxo_map utxoMap;
    
public:
    
    struct UTXOMissingException : public std::runtime_error {
        UTXOMissingException(const RawOutputPointer &pointer) : std::runtime_error(getMessage(pointer).c_str()) {}
        
    private:
        static std::string getMessage(const RawOutputPointer &pointer) {
            std::stringstream ss;
            ss << "Tried to spend missing UTXO " << pointer.hash.GetHex() << ": " << pointer.outputNum;
            return ss.str();
        }
    };
    
    UTXOState(const ParserConfigurationBase &config);
    UTXOState(const UTXOState &) = delete;
    UTXOState &operator=(const UTXOState &) = delete;
    UTXOState(UTXOState &&) = delete;
    UTXOState &operator=(UTXOState &&) = delete;
    ~UTXOState();
    
    UTXO spendOutput(const RawOutputPointer &outputPointer);
    void addOutput(UTXO utxo, const RawOutputPointer &outputPointer);
};


#endif /* utxo_state_hpp */
