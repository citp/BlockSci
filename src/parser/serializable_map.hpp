//
//  serializable_map.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 11/13/17.
//

#ifndef serializable_map_hpp
#define serializable_map_hpp

#include <stdio.h>

#include <blocksci/hash.hpp>

#include <google/dense_hash_map>

#include <sstream>
#include <fstream>
#include <future>

template<typename Key, typename Value>
class SerializableMap {
    using Map = google::dense_hash_map<Key, Value, std::hash<Key>>;
    Map map;
    
public:
    
    struct MissingKeyException : public std::runtime_error {
        MissingKeyException() : std::runtime_error("Tried to remove missing key") {}
    };
    
    SerializableMap(const Key &deletedKey, const Key emptyKey) {
        map.set_deleted_key(deletedKey);
        map.set_empty_key(emptyKey);
    }
    
    bool unserialize(const std::string &path) {
        std::fstream file{path, std::fstream::in | std::fstream::binary};
        if (file.is_open()) {
            typename Map::NopointerSerializer serializer;
            return map.unserialize(serializer, &file);
        }
        return false;
    }
    
    bool serialize(const std::string &path) {
        std::ofstream file{path, std::fstream::out | std::fstream::binary};
        typename Map::NopointerSerializer serializer;
        return map.serialize(serializer, &file);
    }
    
    Value erase(const Key &key) {
        auto it = map.find(key);
        if (it == map.end()) {
            throw MissingKeyException();
        }
        
        Value value = it->second;
        map.erase(it);
        return value;
    }
    
    void add(const Key &key, const Value &value) {
        map.insert(std::make_pair(key, value));
    }
};

#endif /* serializable_map_hpp */
