#include "FileController.hpp"

FileController::FileController(std::string path, Mode mode) : _mode(mode) {
    _fd = -1;
    _metaData = NULL;
    _path = toAbsolutePath(path);
    if (_path.empty()) {
        throw ErrorHandler("Error: path is empty.", ErrorHandler::ALERT, "FileController::FileController");
    }
    if (_mode == READ) {
        _type = checkType(_path);
        if (_type == FileController::FILE) {
            _metaData = getMetaData(_path);
            _fd = open(_path.c_str(), O_RDONLY);
        } else if (_type == FileController::DIRECTORY) {
            _metaData = getMetaData(_path);
            getFilesOfFolder(_path, _filesMetaData);
        } else {
            throw ErrorHandler("Error: File Not Found.", ErrorHandler::ALERT, "FileController::FileController");
        }
    } else {
        _type = FileController::FILE;
        _fd = open(_path.c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG);
    }
}

FileController::~FileController() {
    delete _metaData;
    std::vector<FileMetaData*>::iterator iter;
    for (iter = _filesMetaData.begin(); iter != _filesMetaData.end(); ++iter) {
        delete *(iter);
    }
    if (_fd != -1) {
        close(_fd);
    }
}

void FileController::getFilesOfFolder(std::string& path, std::vector<FileMetaData*>& vector) {
    struct dirent* dp;
    DIR* dir = opendir(path.c_str());
    while ((dp = readdir(dir)) != NULL) {
        std::string subpath;
        if (path.at(path.length() - 1) == '/') {
            subpath = path + std::string(dp->d_name, dp->d_namlen);
        } else {
            subpath = path + "/" + std::string(dp->d_name, dp->d_namlen);
        }
        vector.push_back(getMetaData(subpath));
    }
    closedir(dir);
}

std::string& FileController::toAbsolutePath(std::string& path) {
    if (path.empty() == false) {
        if (path.at(0) != '/') {
            char* pwd = getcwd(NULL, 0);
            path = std::string(pwd) + "/" + path;
            delete pwd;
        }
    }
    return (path);
}

FileController::Type FileController::modeToType(mode_t mode) {
    if (S_ISREG(mode)) {
        return (FILE);
    } else if (S_ISDIR(mode)) {
        return (DIRECTORY);
    } else {
        return (NOTFOUND);
    }
}

FileController::Type FileController::checkType(std::string path) {
    struct stat buf;
    if (stat(path.c_str(), &buf) == -1) {
        return (NOTFOUND);
    }
    return (modeToType(buf.st_mode));
}

FileController::FileMetaData* FileController::getMetaData(std::string path) {
    char timeBuffer[20];
    FileController::FileMetaData* rtn = NULL;
    struct stat buf;
    struct passwd* udata;
    struct tm *timeinfo;
    if (stat(path.c_str(), &buf) == -1 || !(S_ISREG(buf.st_mode) || S_ISDIR(buf.st_mode))) {
        return (NULL);
    }
    rtn = new FileController::FileMetaData;
    udata = getpwuid(buf.st_uid);
	timeinfo = std::localtime(&(buf.st_mtimespec.tv_sec));
	std::strftime(timeBuffer, 20, "%Y/%m/%d %H:%M:%S", timeinfo);
    size_t find = path.rfind("/");
    if (find == std::string::npos) {
        rtn->name = path;
    } else {
        rtn->name = path.substr(find + 1);    
    }
    if (udata != NULL) {
        rtn->userid = std::string(udata->pw_name);
    } else {
        rtn->userid = std::string("");
    }
    rtn->type = modeToType(buf.st_mode);
    rtn->generateTime = std::string(timeBuffer);
    rtn->size = buf.st_size;
    return (rtn);
}

int FileController::getFilesSize(void) const {
    if (_mode == READ) {
        return (_filesMetaData.size());
    } else {
        return (-1);
    }
}

FileController::FileMetaData* FileController::getFiles(int i) const {
    if (i < 0) {
        return (_metaData);
    } else if (_filesMetaData.size() <= size_t(i)) {
        return (NULL);
    } else {
        return (_filesMetaData[i]);
    }
}

FileController::Type FileController::getType(void) const {
    return (_type);
}

int FileController::getFd(void) const {
    return (_fd);
}
long FileController::length(void) const {
    if (_mode == READ) {
        return (_metaData->size);
    } else {
        return (-1);
    }
}

bool FileController::del(void) {
    if (_type != NOTFOUND && _fd != -1) {
        close(_fd);
        _fd = -1;
        if (unlink(_path.c_str()) != -1) {
            return (true);
        } else {
            throw ErrorHandler("Error: File Delete Eror.", ErrorHandler::ALERT, "FileController::del");
        }
    }
    return (false);
}
