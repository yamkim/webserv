#ifndef HTMLBODY_HPP
#define HTMLBODY_HPP

#include <string>
#include "FileController.hpp"
#include <sstream>
#include <iomanip>

class HTMLBody {
    public:
        static std::string getStaticHTML(int statusCode) {
            std::string statusMsg;
            if (statusCode == 404) {
                statusMsg = "404 Not Found";
            }

            std::string serverName = "webserv/0.0.1";
            std::string ret;
            ret = "<html>";
            ret += "<head><title>" + statusMsg + "</title></head>";
            ret += "<body>";
            ret += "<center><h1>" + statusMsg + "</h1></center>";
            ret += "<hr>";
            ret += "<center>" + serverName + "</center>";
            ret += "</body>";
            ret += "</html>";
            return ret;
        }

        static std::string getAutoIndexBody(std::string root, std::string path) {
            FileController folder = FileController(root + path, FileController::READ);
            std::stringstream stringBuffer;
            stringBuffer << "<html>";
            stringBuffer << "<head><title>Index of /" << path << "</title></head>";
            stringBuffer << "<body>";
            stringBuffer << "<h1>Index of /" << path << "</h1><hr><pre>" << std::endl;
            if (folder.getType() == FileController::DIRECTORY) {
                for (int i = 0; i < folder.getFilesSize(); ++i) {
                    if (folder.getFiles(i)->name == std::string(".")) {
                        continue ;
                    }
                    std::string fileName;
                    if (folder.getFiles(i)->type == FileController::DIRECTORY) {
                        fileName = folder.getFiles(i)->name + std::string("/");
                    } else {
                        fileName = folder.getFiles(i)->name;
                    }
                    stringBuffer << "<a href=\"" << fileName << "\">";
                    stringBuffer << std::setw(53) << std::setfill(' ');
                    stringBuffer << std::left << (fileName + std::string("</a>"));
                    stringBuffer << std::right;
                    stringBuffer << folder.getFiles(i)->generateTime;
                    stringBuffer << std::setw(20) << std::setfill(' ');
                    if (folder.getFiles(i)->type == FileController::DIRECTORY) {
                        stringBuffer << "-" << std::endl;
                    } else {
                        stringBuffer << folder.getFiles(i)->size << std::endl;
                    }
                }
            }
            stringBuffer << "</pre><hr></body>";
            stringBuffer << "</html>";
            return (stringBuffer.str());
        }
};
#endif