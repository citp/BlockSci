//
//  safe_mem_reader.hpp
//  blocksci_parser
//
//  Created by Harry Kalodner on 9/26/17.
//

#ifndef safe_mem_reader_hpp
#define safe_mem_reader_hpp

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem/path.hpp>

class SafeMemReader {
public:
    typedef boost::iostreams::mapped_file_source::iterator iterator;
    typedef boost::iostreams::mapped_file_source::size_type size_type;
    typedef std::iterator_traits<iterator>::difference_type difference_type;
    
    explicit SafeMemReader(const boost::filesystem::path &path) {
        fileMap.open(path);
        begin = fileMap.begin();
        end = fileMap.end();
        pos = begin;
    }

    bool has(difference_type n) {
        return n <= std::distance(pos, end);
    }
    
    template<typename Type>
    Type readNext() {
        constexpr auto size = sizeof(Type);
        if (!has(size)) {
            throw std::out_of_range("Tried to read past end of file");
        }
        Type val;
        memcpy(&val, pos, size);
        pos += size;
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
    
    void advance(size_type n) {
        if (!has(n)) {
            throw std::out_of_range("Tried to advance past end of file");
        }
        pos += n;
    }
    
    void reset() {
        pos = begin;
    }
    
    void reset(size_type n) {
        if (begin + n > end) {
            throw std::out_of_range("Tried to reset out of file");
        }
        pos = begin + n;
    }
    
    size_type offset() {
        return std::distance(begin, pos);
    }
    
    boost::iostreams::mapped_file_source::iterator unsafePos() {
        return pos;
    }
    
protected:
    boost::iostreams::mapped_file_source fileMap;
    iterator pos;
    iterator begin;
    iterator end;
};

#endif /* safe_mem_reader_hpp */
