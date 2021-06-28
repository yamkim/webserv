#ifndef FILECONTROLLER_HPP
#define FILECONTROLLER_HPP

#include <string>
#include <ctime>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

class FileController {
    public:
        typedef enum e_Type {FILE, DIRECTORY, NOTFOUND} Type;
        typedef enum e_Mode {READ, WRITE} Mode;
        typedef struct s_FileMetaData {
            std::string name;
            std::string userid;
            FileController::Type type;
            std::string generateTime;
            long size;
        } FileMetaData;
    private:
        int _fd;
        Type _type;
        const Mode _mode;
        std::string _path;
        FileMetaData* _metaData;
        std::vector<FileMetaData*> _filesMetaData;
        FileController();
        static inline void getFilesOfFolder(std::string& path, std::vector<FileMetaData*>& vector);
        static inline std::string& toAbsolutePath(std::string& path);
        static inline Type modeToType(mode_t mode);
        static FileMetaData* getMetaData(std::string path);
    public:
        FileController(std::string path, Mode mode);
        ~FileController();
        static Type typeCheck(std::string path);
        int getFilesSize(void) const;
        FileMetaData* getFiles(int i) const;
        Type getType(void) const;
        int getFd(void) const;
        long length(void) const;
        bool del(void);
};

#endif
