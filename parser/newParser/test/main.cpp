#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stack>

class TestData {
protected:
    TestData();
    std::string _rawData;
    std::string _fileName;

public:
    TestData(const std::string& fileName) : _fileName(fileName) {
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

class TestFunction {
    public:
        static void findBlockSet(const std::string& buf, std::stack<int>st, std::vector<std::pair<std::string, std::size_t> >& vec, std::size_t& pos) {
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

        static std::string getBlockContent(const std::string& buf, std::size_t& pos) {
            std::vector<std::pair<std::string, std::size_t> > blockSet;
            std::stack<int> st;

            findBlockSet(buf, st, blockSet, pos);
            std::size_t blockBeg = blockSet.begin()->second;
            std::size_t blockEnd = (blockSet.end() - 1)->second;
            pos = blockEnd + 1;
            std::cout << "beg: " << blockBeg << ", end: " << blockEnd << std::endl;

            return buf.substr(blockBeg + 1, blockEnd - blockBeg - 1);
        }
};

void testFindBlockSet(const std::string& rawData) {
    std::size_t pos = 0;
    std::vector<std::pair<std::string, std::size_t> > blockSet;
    std::stack<int> st;
    std::cout << "rawData: " << rawData << std::endl;
    std::cout << "before find block set: " << pos << std::endl;
    TestFunction::findBlockSet(rawData, st, blockSet, pos);
    pos = (blockSet.end() - 1)->second + 1;
    std::vector<std::pair<std::string, std::size_t> >::iterator iter;
    for (iter = blockSet.begin(); iter != blockSet.end(); ++iter) {
        std::cout << "iter[type]: " << (*iter).first << ", iter[pos]: " << (*iter).second << std::endl;
    }
    std::cout << "after find block set: " << pos << std::endl;
}

void testGetBlockContent(const std::string& rawData) {
    std::size_t pos = 0;
    std::cout << TestFunction::getBlockContent(rawData, pos) << std::endl;
    std::cout << TestFunction::getBlockContent(rawData, pos) << std::endl;
    std::cout << TestFunction::getBlockContent(rawData, pos) << std::endl;
}

int main() {
    try {
        TestData test("nginx2.conf");
        std::string rawData = test.getRawData();
       // std::size_t i = 0;
        std::string identifier;

        //testFindBlockSet(rawData);
        testGetBlockContent(rawData);


    } catch (const std::string& e) {
        std::cout << e << std::endl;
    }


}