//
//  file_writer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/12/17.
//
//

#ifndef file_writer_hpp
#define file_writer_hpp

#include <boost/filesystem/fstream.hpp>

struct SimpleFileWriter {
protected:
    uint64_t lastDataPos;
    boost::filesystem::fstream file;
public:
    
    uint64_t getLastPos() const { return lastDataPos; }
    
    SimpleFileWriter(boost::filesystem::path path) {
        auto mainParams = std::fstream::out | std::fstream::binary;
        auto extraParams = std::fstream::ate | std::fstream::in;
        path = path.concat(".dat");
        
        file.open(path, mainParams | extraParams);
        if (!file.is_open())
        {
            // create
            file.open(path, mainParams);
            
            // close
            if (file.is_open())
                file.close();
            
            // re-open
            file.open(path, mainParams | extraParams);
        }
        lastDataPos = static_cast<size_t>(file.tellp());
    }
    
    ~SimpleFileWriter() {
        file.close();
    }
    
    template<typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    void writeImp(const T &t) {
        file.write(reinterpret_cast<const char *>(&t), sizeof(T));
        lastDataPos += sizeof(T);
    }
    
    size_t size() const {
        return lastDataPos;
    }
    
    void flush() {
        file.flush();
    }
};

struct ArbitraryFileWriter : public SimpleFileWriter {
    using SimpleFileWriter::SimpleFileWriter;
    
    template<typename T>
    void write(const T &t) {
        writeImp(t);
    }
    
    template<class It>
    void write(It it, It end) {
        uint32_t distance = std::distance(it, end);
        writeImp(distance);
        while (it != end) {
            writeImp(*(it++));
        }
    }
};

template <typename T, typename = typename std::enable_if<std::is_trivially_copyable<T>::value>::type>
struct FixedSizeFileWriter : public SimpleFileWriter {
    using SimpleFileWriter::SimpleFileWriter;
    
    void expandToFit(uint32_t size) {
        file.seekp(sizeof(T) * size - 1);
        file.write("", 1);
    }
    
    void write(const T &t) {
        SimpleFileWriter::writeImp(t);
    }
    
    template<typename K>
    void updateData(uint32_t scriptNum, size_t offset, const K &t) {
        file.seekp((scriptNum - 1) * sizeof(T) + offset);
        file.write(reinterpret_cast<const char *>(&t), sizeof(t));
        file.seekp(this->lastDataPos);
    }
    
    T read(uint32_t scriptNum) {
        file.flush();
        T ret;
        file.seekg((scriptNum - 1) * sizeof(T));
        file.read(reinterpret_cast<char *>(&ret), sizeof(T));
        file.seekg(this->lastDataPos);
        return ret;
    }
    
    size_t size() const {
        return SimpleFileWriter::size() / sizeof(T);
    }
};

template <size_t indexCount>
using FileIndex = std::array<uint64_t, indexCount>;

template <size_t indexCount>
struct IndexedFileWriter : public ArbitraryFileWriter {
private:
    using IndexType = uint64_t;
    FixedSizeFileWriter<FileIndex<indexCount>> indexFile;
public:
    
    IndexedFileWriter(boost::filesystem::path pathPrefix) : ArbitraryFileWriter(boost::filesystem::path{pathPrefix}.concat("_data")), indexFile(boost::filesystem::path{pathPrefix}.concat("_index")) {}
    
    void writeIndexGroup() {
        FileIndex<indexCount> fileIndex;
        fileIndex[0] = getLastPos();
        for(size_t i = 1; i < indexCount; i++) {
            fileIndex[i] = 0;
        }
        indexFile.write(fileIndex);
    }
    
    template<size_t indexNum>
    void updateIndexGroup(uint32_t addressNum) {
        static_assert(indexNum < indexCount, "Index out of range");
        indexFile.updateData(addressNum, indexNum, getLastPos());
    }
    
    size_t size() const {
        return indexFile.size();
    }
    
    void flush() {
        ArbitraryFileWriter::flush();
        indexFile.flush();
    }
};

#endif /* file_writer_hpp */
