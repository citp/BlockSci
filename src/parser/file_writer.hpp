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
    
    template<typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    void writeImp(const T &t) {
        file.write(reinterpret_cast<const char *>(&t), sizeof(T));
        lastDataPos += sizeof(T);
    }
    
    template <typename T>
    T read(size_t offset) {
        T ret;
        file.seekg(static_cast<std::streamoff>(offset));
        file.read(reinterpret_cast<char *>(&ret), sizeof(T));
        file.seekg(static_cast<std::streamoff>(lastDataPos));
        return ret;
    }
    
    template<typename K>
    void update(size_t offset, const K &t) {
        file.seekp(static_cast<std::streamoff>(offset));
        file.write(reinterpret_cast<const char *>(&t), sizeof(t));
        file.seekp(static_cast<std::streamoff>(lastDataPos));
    }
    
    void expandToFit(std::streamoff size) {
        file.seekp(size - 1);
        file.write("", 1);
    }
    
    size_t size() const {
        return lastDataPos;
    }
    
    void flush() {
        file.flush();
    }
};

struct ArbitraryFileWriter : SimpleFileWriter {
    using SimpleFileWriter::SimpleFileWriter;
    
    template<typename T>
    void write(const T &t) {
        writeImp(t);
    }
    
    template<class It>
    void write(It it, It end) {
        uint32_t distance = static_cast<uint32_t>(std::distance(it, end));
        writeImp(distance);
        while (it != end) {
            writeImp(*(it++));
        }
    }
};

template <typename T, typename = typename std::enable_if<std::is_trivially_copyable<T>::value>::type>
class FixedSizeFileWriter {
    SimpleFileWriter dataFile;
    
public:
    
    FixedSizeFileWriter(const boost::filesystem::path &path) : dataFile(path) {}
    
    void expandToFit(uint32_t size) {
        dataFile.expandToFit(sizeof(T) * size);
    }
    
    void write(const T &t) {
        dataFile.writeImp(t);
    }
    
    template<typename K>
    void updateData(uint32_t index, size_t offset, const K &t) {
        dataFile.update(index * sizeof(T) + offset, t);
    }
    
    T read(uint32_t index) {
        return dataFile.read<T>(index * sizeof(T));
    }
    
    size_t size() const {
        return dataFile.size() / sizeof(T);
    }
};

template <size_t indexCount>
using FileIndex = std::array<uint64_t, indexCount>;

template <size_t indexCount>
struct IndexedFileWriter {
private:
    using IndexType = uint64_t;
    ArbitraryFileWriter dataFile;
    FixedSizeFileWriter<FileIndex<indexCount>> indexFile;
public:
    
    IndexedFileWriter(boost::filesystem::path pathPrefix) : dataFile(boost::filesystem::path{pathPrefix}.concat("_data")), indexFile(boost::filesystem::path{pathPrefix}.concat("_index")) {}
    
    void writeIndexGroup() {
        FileIndex<indexCount> fileIndex;
        fileIndex[0] = dataFile.getLastPos();
        for(size_t i = 1; i < indexCount; i++) {
            fileIndex[i] = 0;
        }
        indexFile.write(fileIndex);
    }
    
    template<typename T>
    void write(const T &t) {
        dataFile.writeImp(t);
    }
    
    void expandToFit(size_t length, uint32_t items) {
        dataFile.expandToFit(length);
        indexFile.expandToFit(items);
    }
    
    template<size_t indexNum>
    void updateIndexGroup(uint32_t addressNum) {
        static_assert(indexNum < indexCount, "Index out of range");
        indexFile.updateData(addressNum, indexNum, dataFile.getLastPos());
    }
    
    size_t size() const {
        return indexFile.size();
    }
    
    void flush() {
        dataFile.flush();
        indexFile.flush();
    }
};

#endif /* file_writer_hpp */
