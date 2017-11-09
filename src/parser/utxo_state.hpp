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

#include <boost/functional/hash.hpp>
#include <boost/filesystem/path.hpp>

#include <google/dense_hash_map>

#include <sstream>
#include <future>

class UTXOState {
    using utxo_map = google::dense_hash_map<RawOutputPointer, UTXO, boost::hash<RawOutputPointer>>;
    
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
    
    UTXOState();
    
    bool unserialize(const boost::filesystem::path &path);
    bool serialize(const boost::filesystem::path &path);
    
    UTXO spendOutput(const RawOutputPointer &outputPointer);
    void addOutput(UTXO utxo, const RawOutputPointer &outputPointer);
};


#endif /* utxo_state_hpp */
