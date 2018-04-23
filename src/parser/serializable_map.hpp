//
//  serializable_map.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 11/13/17.
//

#ifndef serializable_map_hpp
#define serializable_map_hpp

#include <google/dense_hash_map>

#include <string>
#include <fstream>

template<typename Key, typename Value>
class SerializableMap {
    using Map = google::dense_hash_map<Key, Value, std::hash<Key>>;
    Map map;
    
public:
    
    using value_type = typename Map::value_type;
    using iterator = typename Map::iterator;
    using const_iterator = typename Map::const_iterator;
    using size_type = typename Map::size_type;
    
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
    
    iterator begin() {
        return map.begin();
    }
    
    const_iterator begin() const {
        return map.begin();
    }
    
    iterator end() {
        return map.end();
    }
    
    const_iterator end() const {
        return map.end();
    }
    
    size_type size() const {
        return map.size();
    }
    
    void clear_no_resize() {
        map.clear_no_resize();
    }
    
    void swap(SerializableMap &other) {
        map.swap(other.map);
    }
    
    void resize(size_type size) {
        map.resize(size);
    }
    
    void erase(iterator pos) {
        map.erase(pos);
    }
    
    template <typename T>
    using without_const = std::add_pointer_t<std::remove_const_t<std::remove_pointer_t<T>>>;
    
    void erase(const_iterator it) {
        auto table = const_cast<without_const<decltype(it.ht)>>(it.ht);
        auto pos = const_cast<without_const<decltype(it.pos)>>(it.pos);
        auto end = const_cast<without_const<decltype(it.end)>>(it.end);
        iterator it2(table, pos, end, false);
        map.erase(it2);
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
    
    const_iterator find(const Key &key) const {
        return map.find(key);
    }
    
    iterator find(const Key &key) {
        return map.find(key);
    }
    
    void add(const Key &key, const Value &value) {
        map.insert(std::make_pair(key, value));
    }
};

#endif /* serializable_map_hpp */
