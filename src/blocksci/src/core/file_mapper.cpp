//
//  file_mapper.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/22/18.
//

#include <blocksci/core/file_mapper.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/device/mapped_file.hpp>


namespace {
    boost::iostreams::mapped_file::mapmode getMapMode(blocksci::AccessMode mode) {
        switch (mode) {
            case blocksci::AccessMode::readwrite:
                return boost::iostreams::mapped_file::mapmode::readwrite;
            case blocksci::AccessMode::readonly:
                return boost::iostreams::mapped_file::mapmode::readonly;
        }
        assert(false);
        return boost::iostreams::mapped_file::mapmode::readonly;
    }
}


namespace blocksci {
    SimpleFileMapperBase::SimpleFileMapperBase(const std::string &path_, AccessMode mode) : file(std::make_unique<boost::iostreams::mapped_file>()), fileEnd(0), path(path_ + ".dat"), fileMode(mode) {
        if (boost::filesystem::exists(path)) {
            openFile(fileSize());
        }
    }

    SimpleFileMapperBase::SimpleFileMapperBase(SimpleFileMapperBase &&other) = default;

    SimpleFileMapperBase::~SimpleFileMapperBase() = default;

    void SimpleFileMapperBase::openFile(size_t size) {
        fileEnd = size;
        if (fileEnd != 0) {
            file->open(path, getMapMode(fileMode));
            const_data = file->const_data();
        } else {
            const_data = nullptr;
        }
    }

    bool SimpleFileMapperBase::isGood() const {
        return file->is_open();
    }

    void SimpleFileMapperBase::reload() {
        if (boost::filesystem::exists(path)) {
            auto newSize = fileSize();
            if (newSize != fileEnd) {
                if (file->is_open()) {
                    file->close();
                }
                openFile(newSize);
            }
        } else {
            if (file->is_open()) {
                file->close();
            }
            fileEnd = 0;
            const_data = nullptr;
        }
    }

    size_t SimpleFileMapperBase::fileSize() const {
        return boost::filesystem::file_size(path);
    }

    SimpleFileMapper<AccessMode::readwrite>::SimpleFileMapper(const std::string &path) : SimpleFileMapperBase(std::move(path), AccessMode::readwrite), writePos(size()) {
        dataPtr = file->data();
    }

    void SimpleFileMapper<AccessMode::readwrite>::reload() {
        clearBuffer();
        SimpleFileMapperBase::reload();
        dataPtr = file->data();
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

    void SimpleFileMapper<AccessMode::readwrite>::clearBuffer() {
        if (buffer.size() > 0) {
            if (!file->is_open()) {
                boost::iostreams::mapped_file_params params{path};
                params.new_file_size = static_cast<decltype(params.new_file_size)>(buffer.size());
                params.flags = boost::iostreams::mapped_file::readwrite;
                file->open(params);
            } else {
                file->resize(static_cast<int64_t>(fileEnd + buffer.size()));
            }
            memcpy(file->data() + fileEnd, buffer.data(), buffer.size());
            fileEnd += buffer.size();
            buffer.clear();
        }
    }
} // namespace blocksci
