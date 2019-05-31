//
//  file_mapper.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/23/17.
//
//

#include "file_mapper.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace blocksci;

namespace {
    boost::iostreams::mapped_file::mapmode getMapMode(AccessMode mode) {
        switch (mode) {
            case AccessMode::readwrite:
                return boost::iostreams::mapped_file::mapmode::readwrite;
            case AccessMode::readonly:
                return boost::iostreams::mapped_file::mapmode::readonly;
        }
        assert(false);
        return boost::iostreams::mapped_file::mapmode::readonly;
    }
}

SimpleFileMapperBase::SimpleFileMapperBase(boost::filesystem::path path_, AccessMode mode) : fileEnd(0), path(path_), fileMode(mode) {
    path += ".dat";
    
    if (boost::filesystem::exists(path)) {
        openFile(fileSize());
    }
}

void SimpleFileMapperBase::openFile(size_t size) {
    fileEnd = size;
    if (fileEnd != 0) {
        file.open(path, getMapMode(fileMode));
        constData = file.const_data();
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

const char *SimpleFileMapperBase::getDataAtOffset(OffsetType offset) const {
    if (offset == InvalidFileIndex) {
        return nullptr;
    }
    assert(offset < size());
    return constData + offset;
}

size_t SimpleFileMapperBase::fileSize() const {
    return boost::filesystem::file_size(path);
}

char *SimpleFileMapper<AccessMode::readwrite>::getWritePos() {
    if (writePos < fileEnd) {
        return reinterpret_cast<char *>(file.data()) + writePos;
    } else if (writePos < fileEnd + buffer.size()) {
        return reinterpret_cast<char *>(buffer.data()) + (writePos - fileEnd);
    } else {
        return nullptr;
    }
}

size_t SimpleFileMapper<AccessMode::readwrite>::writeSpace() const {
    if (writePos < fileEnd) {
        return fileEnd - writePos;
    } else if (writePos < fileEnd + buffer.size()) {
        return (writePos - fileEnd) - buffer.size();
    } else {
        return 0;
    }
}

bool SimpleFileMapper<AccessMode::readwrite>::write(const char *valuePos, size_t amountToWrite) {
    if (writePos < fileEnd) {
        auto writeAmount = std::min(amountToWrite, writeSpace());
        memcpy(getWritePos(), valuePos, writeAmount);
        amountToWrite -= writeAmount;
        writePos += writeAmount;
        valuePos += writeAmount;
        if (amountToWrite == 0) {
            return false;
        }
    }
    
    if (writePos >= fileEnd && writePos < buffer.size()) {
        auto writeAmount = std::min(amountToWrite, writeSpace());
        memcpy(getWritePos(), valuePos, writeAmount);
        amountToWrite -= writeAmount;
        writePos += writeAmount;
        valuePos += writeAmount;
        
        if (amountToWrite == 0) {
            return false;
        }
    }
    
    assert(writePos == fileEnd + buffer.size());
    
    buffer.insert(buffer.end(), valuePos, valuePos + amountToWrite);
    writePos += amountToWrite;
    bool bufferFull = buffer.size() > maxBufferSize;
    if (bufferFull) {
        clearBuffer();
    }
    return bufferFull;
}

void SimpleFileMapper<AccessMode::readwrite>::clearBuffer() {
    if (buffer.size() > 0) {
        if (!file.is_open()) {
            boost::iostreams::mapped_file_params params{path.native()};
            params.new_file_size = static_cast<decltype(params.new_file_size)>(buffer.size());
            params.flags = boost::iostreams::mapped_file::readwrite;
            file.open(params);
        } else {
            file.resize(static_cast<int64_t>(fileEnd + buffer.size()));
        }
        memcpy(file.data() + fileEnd, buffer.data(), buffer.size());
        fileEnd += buffer.size();
        buffer.clear();
    }
}

char *SimpleFileMapper<AccessMode::readwrite>::getDataAtOffset(OffsetType offset) {
    assert(offset < fileEnd + buffer.size() || offset == InvalidFileIndex);
    if (offset == InvalidFileIndex) {
        return nullptr;
    } else if (offset < fileEnd) {
        return file.data() + offset;
    } else {
        return buffer.data() + (offset - fileEnd);
    }
}

const char *SimpleFileMapper<AccessMode::readwrite>::getDataAtOffset(OffsetType offset) const {
    assert(offset < fileEnd + buffer.size() || offset == InvalidFileIndex);
    if (offset == InvalidFileIndex) {
        return nullptr;
    } else if (offset < fileEnd) {
        return file.const_data() + offset;
    } else {
        return buffer.data() + (offset - fileEnd);
    }
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
            s.seekp(static_cast<long long>(offset - 1));
            s.write("", 1);
        } else {
            boost::filesystem::resize_file(path, offset);
        }
        reload();
    }
}
