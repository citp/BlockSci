//
//  file_mapper.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/22/18.
//

#include <blocksci/core/file_mapper.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

namespace blocksci {
    bool FileInfo::exists() const {
        return boost::filesystem::exists(path);
    }
    
    OffsetType FileInfo::size() const {
        return static_cast<OffsetType>(boost::filesystem::file_size(path));
    }
    
    void FileInfo::resize(OffsetType offset) {
        boost::filesystem::resize_file(path, static_cast<uintmax_t>(offset));
    }
    
    void FileInfo::create(OffsetType offset) {
        assert(!exists());
        boost::filesystem::fstream s{path, boost::filesystem::fstream::out | boost::filesystem::fstream::binary};
        s.seekp(static_cast<int64_t>(offset - 1));
        s.write("", 1);
    }


    void SimpleFileMapper<mio::access_mode::write>::truncate(OffsetType offset) {
        if (offset < SimpleFileMapperBase::size()) {
            buffer.clear();
            fileInfo.resize(offset);
            reload();
        } else if (offset < size()) {
            auto bufferToSave = offset - SimpleFileMapperBase::size();
            buffer.resize(static_cast<size_t>(bufferToSave));
        } else if (offset > size()) {
            clearBuffer();
            if (!fileInfo.exists()) {
                fileInfo.create(offset);
            } else {
                fileInfo.resize(offset);
            }
            SimpleFileMapperBase::reload();
        }
    }

    void SimpleFileMapper<mio::access_mode::write>::clearBuffer() {
        if (buffer.size() > 0) {
            auto oldEnd = size();
            if (!fileInfo.exists()) {
                fileInfo.create(static_cast<OffsetType>(buffer.size()));
            } else {
                fileInfo.resize(oldEnd + static_cast<OffsetType>(buffer.size()));
            }
            memcpy(&file[oldEnd], buffer.data(), buffer.size());
            buffer.clear();
            SimpleFileMapperBase::reload();
        }
    }
} // namespace blocksci
