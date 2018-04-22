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
    SimpleFileMapperBase::SimpleFileMapperBase(boost::filesystem::path path_, AccessMode mode) : fileEnd(0), path(std::move(path_)), fileMode(mode) {
        path += ".dat";
        
        if (boost::filesystem::exists(path)) {
            openFile(fileSize());
        }
    }

    void SimpleFileMapperBase::reload() {
        if (boost::filesystem::exists(path)) {
            auto newSize = fileSize();
            if (newSize != fileEnd) {
                if (file.is_open()) {
                    file.close();
                }
                openFile(newSize);
            }
        } else {
            if (file.is_open()) {
                file.close();
            }
            fileEnd = 0;
        }
    }

    size_t SimpleFileMapperBase::fileSize() const {
        return boost::filesystem::file_size(path);
    }

    void SimpleFileMapper<AccessMode::readwrite>::truncate(OffsetType offset) {
        if (offset < SimpleFileMapperBase::size()) {
            buffer.clear();
            boost::filesystem::resize_file(path, offset);
            reload();
        } else if (offset < size()) {
            auto bufferToSave = offset - SimpleFileMapperBase::size();
            buffer.resize(bufferToSave);
        } else if (offset > size()) {
            clearBuffer();
            if (!boost::filesystem::exists(path)) {
                boost::filesystem::fstream s{path, std::fstream::out | std::fstream::binary};
                s.seekp(static_cast<int64_t>(offset - 1));
                s.write("", 1);
            } else {
                boost::filesystem::resize_file(path, offset);
            }
            reload();
        }
    }
} // namespace blocksci
