
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stack>

class Parser {
protected:
    Parser();
    std::string _rawData;
    std::string _fileName;

public:
    Parser(const std::string& fileName) : _fileName(fileName) {
        _rawData = "";
        std::ifstream readFile;

        readFile.open(this->_fileName);
        if (!readFile.is_open())
            throw std::string("Error: File open error.");
        while (!readFile.eof()) {
            std::string tmp;
            getline(readFile, tmp);
            this->_rawData += tmp;
            this->_rawData += "\n";
        }
        readFile.close();

        if (!isValidBlockSet(this->_rawData)) {
            throw std::string("Error: bracket pair is not valid.");
        }
    }
    virtual ~Parser(){};

    const std::string& getRawData() const{
        return _rawData;
    }

    bool isValidBlockSet (const std::string& buf) {
        std::size_t pos = 0;
        int leftBracketNum = 0;
        int rightBracketNum = 0;

        while (buf[pos]) {
            if (buf[pos] == '{') {
                leftBracketNum++;
            } else if (buf[pos] == '}') {
                rightBracketNum++;
            }
            ++pos;
        }
        return (leftBracketNum == rightBracketNum);
    }

};

class NginxConfig {


};

void skipComment (const std::string& str, std::size_t &commentPos) {
    while (str[commentPos]) {
        if (str[commentPos] != '\n') {
            ++commentPos;
        } else {
            return ;
        }
    }
}

bool isCharinString(const std::string& str, const char c) {
    std::size_t itr = 0;
    while (str[itr]) {
        if (str[itr] == c)
            return true;
        ++itr;
    }
    return false;
} 

std::string	getIdentifier(const std::string str, std::size_t& endPos, std::string delimiter)
{
	size_t wordSize = 0;

	while ((str[endPos] != '\0') && isCharinString(delimiter, str[endPos])) {
        ++endPos;
    }
    size_t begPos = endPos;
	while ((str[endPos] != '\0') && !isCharinString(delimiter, str[endPos])) {
        ++wordSize;
        ++endPos;
    }
	return (str.substr(begPos, wordSize));
}

void findBlockSet(const std::string& buf, std::stack<int>st, std::vector<std::pair<std::string, std::size_t> >& vec, std::size_t& pos) {
    while (buf[pos] != '\0' && buf[pos] != '{' && buf[pos] != '}') {
        ++pos;
    }
    if (buf[pos] == '{') {
        st.push(1);
        vec.push_back(std::make_pair("{", pos));
    } else if (buf[pos] == '}') {
        if (st.top() == 1) {
            st.pop();
        }
        vec.push_back(std::make_pair("}", pos));
    }
    if (st.empty()) {
        return ;
    }
    pos += 1;
    findBlockSet(buf, st, vec, pos);
}

std::string getBlockContent(const std::string& buf, std::size_t& pos) {
    std::vector<std::pair<std::string, std::size_t> > blockSet;
    std::stack<int> st;

    findBlockSet(buf, st, blockSet, pos);
    std::size_t blockBeg = blockSet.begin()->second;
    std::size_t blockEnd = (blockSet.end() - 1)->second;
    pos = blockEnd + 1;
    // std::cout << "beg: " << blockBeg << ", end: " << blockEnd << std::endl;

    return buf.substr(blockBeg + 1, blockEnd - blockBeg - 1);
}

std::string leftSpaceTrim(std::string s) {
    const std::string drop = " ";
    return s.erase(0,s.find_first_not_of(drop));
}

std::string rightSpaceTrim(std::string s) {
    const std::string drop = " ";
    return s.erase(s.find_last_not_of(drop)+1);
}

struct locationBlock {
    std::string locationPath;
    std::vector<std::string> locationReturn;
};
struct serverBlock {
    std::string listen;
    std::string server_name;
    struct locationBlock location[2];
};
struct httpBlock {
    std::string charset;
    std::string include;
    std::string default_type;

    struct serverBlock server[2];
};

struct locationBlock setLocationBlock(const std::string& buf, std::string locationPath) {
    std::size_t pos = 0;
    struct locationBlock ret;

    ret.locationPath = locationPath;
    while (buf[pos]) {
        // line 위치 기록중
        std::string tmpLine = getIdentifier(buf, pos, "\n");
        if (tmpLine.empty()) {
            continue ;
        }
        std::size_t delimitPos = 0;
        std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
        if (tmpDir == "listen") {
            ret.locationReturn.push_back(leftSpaceTrim(getIdentifier(tmpLine, delimitPos, " ")));
            ret.locationReturn.push_back(leftSpaceTrim(getIdentifier(tmpLine, delimitPos, ";")));
        } 
        // std::cout << "identifier[location]: " << tmpDir << std::endl;
    }
    // std::cout << "location.locationPath: " << ret.locationPath << std::endl;
    return ret;
}


struct serverBlock setServerBlock(const std::string& buf) {
    std::size_t pos = 0;
    struct serverBlock ret;

    while (buf[pos]) {
        // line 위치 기록중
        std::string tmpLine = getIdentifier(buf, pos, "\n");
        if (tmpLine.empty()) {
            continue ;
        }
        std::size_t delimitPos = 0;
        std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
        if (tmpDir == "listen") {
            ret.listen = leftSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
        } else if (tmpDir == "server_name") {
            ret.server_name = leftSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
        } else if (tmpDir == "location") {
            std::string locationPath = getIdentifier(tmpLine, delimitPos, " ");
            std::string blockContent = getBlockContent(buf, delimitPos);
            pos = delimitPos;

            ret.location[0] = setLocationBlock(blockContent, locationPath);
        }
        // std::cout << "identifier[server]: " << tmpDir << std::endl;
    }
    // std::cout << "server.listen: " << ret.listen << std::endl;
    // std::cout << "server.server_name: " << ret.server_name << std::endl;
    return ret;
}

// 독립적으로 사용됨. pos를 받고 진행 상황을 기록할 필요 없음
struct httpBlock setHttpBlock(const std::string& buf) {
    struct httpBlock ret;
    std::size_t pos = 0;

    while (buf[pos]) {
        // line 위치 기록중
        std::string tmpLine = getIdentifier(buf, pos, "\n");
        if (tmpLine.empty()) {
            continue ;
        }
        std::size_t delimitPos = 0;
        std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
        if (tmpDir == "charset") {
            ret.charset = leftSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
        } else if (tmpDir == "include") {
            ret.include = leftSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
        } else if (tmpDir == "server") {
            std::string blockContent = leftSpaceTrim(getBlockContent(buf, delimitPos));
            pos = delimitPos;
            ret.server[0] = setServerBlock(blockContent);
        }
        // std::cout << "identifier[http]: " << tmpDir << std::endl;
    }

    // std::cout << "http charset: " << ret.charset << std::endl;
    // std::cout << "http include: " << ret.include << std::endl;
    return ret;
}

int main() {
    // std::cout << nginx.getRawData() << std::endl;

    Parser nginx("nginx.conf");
    std::string nginxRawData = nginx.getRawData();
    std::size_t pos = 0;
    std::string identifier;
    struct httpBlock http;

    std::vector<std::vector<std::string> > totalLineIdentifier;
    while (nginxRawData[pos]) {
        std::string tmpLine = getIdentifier(nginxRawData, pos, "\n");
        if (tmpLine.empty()) {
            continue ;
        }
        // std::cout << "one line: " << tmpLine << std::endl; 

        std::size_t delimitPos = 0;
        std::vector<std::string> oneLineIdentifier;

        // 일단 한 번 쪼개보고, identifer가 블록인지 블록이 아닌지 판단
        // 블록이면, 블록 형태의 structure, 아니면 none 블록 형태의 structure로
        std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
        if (tmpDir == "http") {
            std::string blockContent = getBlockContent(nginxRawData, delimitPos);
            http = setHttpBlock(blockContent);
            pos = delimitPos; // 블록 이후의 pos
        }
    }

    std::cout << "http::charset: " << http.charset << std::endl;
    std::cout << "http::include: " << http.include << std::endl;
    std::cout << "http::default_type: " << http.default_type << std::endl;
    std::cout << "http::server::listen " << http.server[0].listen << std::endl;
    std::cout << "http::server::server_name " << http.server[0].server_name << std::endl;
    std::cout << "http::server::location::path " << http.server[0].location[0].locationPath << std::endl;


    return (0);
}