#ifndef FILECONTROLLER_HPP
#define FILECONTROLLER_HPP
// NOTE : FileController : 파일 입/출력을 위한 클래스
/**
 * HTTP Request : 임시 파일 혹은 파일 업로드 시 파일 작성 (현재 미구현)
 * HTTP Response : 요청 파일을 읽어오는 데 사용
 * 현재 문제점 : C++ 표준 파일 입출력 클래스가 파일과 폴더를 구분하지 못함
 *            index 파일이 없을 때 autoindex 기능을 켜면 폴더를 조회해야 하는데 못함 (해당기능이 없음)
 *
 **/

#include <fstream>
#include <string>

class FileController {
    private:
        std::fstream _file;
    public:
        FileController();
        ~FileController();
        void open(std::string& fileName, std::ios_base::openmode mode);
        bool isOpen(void);
        int length(void);
        int read(char* buf, int bugLength);
        int write(char* buf, int bugLength);
};

#endif
