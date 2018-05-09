//
//  safe_mem_reader.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 9/26/17.
//

#ifndef safe_mem_reader_hpp
#define safe_mem_reader_hpp

#include <mio/mmap.hpp>

inline unsigned int variableLengthIntSize(uint64_t nSize) {
    if (nSize < 253)             return sizeof(unsigned char);
    else if (nSize <= std::numeric_limits<unsigned short>::max()) return sizeof(unsigned char) + sizeof(unsigned short);
    else if (nSize <= std::numeric_limits<unsigned int>::max())  return sizeof(unsigned char) + sizeof(unsigned int);
    else                         return sizeof(unsigned char) + sizeof(uint64_t);
}

class SafeMemReader {
public:
    using iterator = mio::mmap_source::const_iterator;
    using size_type = mio::mmap_source::size_type;
    using difference_type = mio::mmap_source::difference_type;
    
    explicit SafeMemReader(std::string path_) : path(std::move(path_)) {
        std::error_code error;
        fileMap.map(path, 0, mio::map_entire_file, error);
        if(error) { throw error; }
        
        begin = fileMap.begin();
        end = fileMap.end();
        pos = begin;
    }
    
    std::string getPath() const {
        return path;
    }

    bool has(difference_type n) {
        return n <= std::distance(pos, end);
    }
    
    template<typename Type>
    Type readNext() {
        auto val = peakNext<Type>();
        pos += sizeof(val);
        return val;
    }
    
    template<typename Type>
    Type peakNext() {
        constexpr auto size = sizeof(Type);
        if (!has(size)) {
            throw std::out_of_range("Tried to read past end of file");
        }
        Type val;
        memcpy(&val, pos, size);
        return val;
    }
    
    // reads a variable length integer.
    // See the documentation from here:  https://en.bitcoin.it/wiki/Protocol_specification#Variable_length_integer
    uint32_t readVariableLengthInteger() {
        auto v = readNext<uint8_t>();
        if ( v < 0xFD ) { // If it's less than 0xFD use this value as the unsigned integer
            return static_cast<uint32_t>(v);
        } else if (v == 0xFD) {
            return static_cast<uint32_t>(readNext<uint16_t>());
        } else if (v == 0xFE) {
            return readNext<uint32_t>();
        } else {
            return static_cast<uint32_t>(readNext<uint64_t>()); // TODO: maybe we should not support this here, we lose data
        }
    }
    
    void advance(difference_type n) {
        if (!has(n)) {
            throw std::out_of_range("Tried to advance past end of file");
        }
        pos += n;
    }
    
    void reset() {
        pos = begin;
    }
    
    void reset(difference_type n) {
        if (begin + n > end) {
            throw std::out_of_range("Tried to reset out of file");
        }
        pos = begin + n;
    }
    
    difference_type offset() {
        return std::distance(begin, pos);
    }
    
    iterator unsafePos() {
        return pos;
    }
    
protected:
    mio::mmap_source fileMap;
    std::string path;
    iterator pos;
    iterator begin;
    iterator end;
};

#endif /* safe_mem_reader_hpp */
