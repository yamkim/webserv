#include "FileController.hpp"

FileController::FileController() {}

FileController::~FileController() {
    _file.close();
}

void FileController::open(std::string& fileName, std::ios_base::openmode mode) {
    _file.open(fileName, mode | std::ios::binary);
}

bool FileController::isOpen(void) {
    return (_file.is_open());
}

int FileController::length(void) {
    int size = -1;
    if (_file.is_open()) {
        _file.seekg (0, _file.end);
        size = _file.tellg();
        _file.seekg (0, _file.beg);
    }
    return (size);
}

int FileController::read(char* buffer, int bufferLength) {
    _file.read(buffer, bufferLength);
    return (_file.gcount());
}

int FileController::write(char* buffer, int bufferLength) {
    _file.write(buffer, bufferLength);
    if (_file.fail()) {
        return (-1);
    }
    return (bufferLength);
}

bool FileController::isExist(std::string& fileName) {
    std::fstream file;
    file.open(fileName, std::ios_base::in);
    if (file.is_open()) {
        file.close();
        return (true);
    } else {
        file.close();
        return (false);
    }
}

bool FileController::isFolder(std::string& fileName) {
    // FIXME: 현재 제기능을 못하는 함수입니다.
    if (fileName.at(fileName.length() - 1) == '/') {
        return (true);
    } else {
        return (false);
    }
}
