#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>

#define REQUEST_FILE_NAME

std::string getFileData(const std::string &fileName) {
    std::string data;
    std::ifstream readFile;

    readFile.open(fileName);
    if (!readFile.is_open())
        throw "Error: File open error.";
    while (!readFile.eof()) {
        std::string tmp;
        getline(readFile, tmp);
        data += tmp;
        data += "\n";
        //std::cout << data << std::endl;
    }
    readFile.close();
    return (data);
}

std::string getStringHeadByDelimiter(const std::string &buf, std::size_t &pos, const std::string &needle) {
    std::size_t found = buf.find(needle, pos);
    if (found == std::string::npos)
        return "";
    std::string strHead = buf.substr(pos, found - pos);
    pos = found + needle.size();
    return strHead;
}


std::map<std::string, std::string> getHttpMap(const std::string &reqBuf) {
    std::map<std::string, std::string> reqHeaders;
    std::size_t pos = 0;

    std::string method = getStringHeadByDelimiter(reqBuf, pos, "\n");
    reqHeaders["First-Line"] = method;

    std::string key, value;
    while ((key = getStringHeadByDelimiter(reqBuf, pos, ": ")) != "") {
        value = getStringHeadByDelimiter(reqBuf, pos, "\n");
        reqHeaders[key] = value;
    }
    return reqHeaders;
}

int main(void) {
    std::string buf;
    try {
        buf = getFileData("requestEx.txt");
    } catch (const char * error) {
        std::cout << error << std::endl;
    }

    std::map<std::string, std::string> reqHeaders = getHttpMap(buf);
    std::map<std::string, std::string>::iterator iter;
    for (iter = reqHeaders.begin(); iter != reqHeaders.end(); ++iter) {
        std::cout << "[" << iter->first << "]: " << iter->second << std::endl;
    }

    return (0);
}
