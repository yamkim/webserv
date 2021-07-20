#include "HTMLBody.hpp"

std::string HTMLBody::getBasicHTMLBody(const std::string& statusMsg) {
    std::string serverName = std::string("webserv/") + std::string(WEBSERV_VERSION);
    std::stringstream ret;
    ret << "<html>";
    ret << "<head><title>" << statusMsg << "</title></head>";
    ret << "<body>";
    ret << "<center><h1>" << statusMsg << "</h1></center>";
    ret << "<hr>";
    ret << "<center>" << serverName << "</center>";
    ret << "</body>";
    ret << "</html>";
    return (ret.str());
}

std::string HTMLBody::getAutoIndexBody(std::string root, std::string path) {
    FileController folder = FileController(root + path, FileController::READ);
    std::stringstream ret;
    ret << "<html>";
    ret << "<head><title>Index of " << path << "</title></head>";
    ret << "<body>";
    ret << "<h1>Index of " << path << "</h1><hr><pre>" << std::endl;
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
        ret << "<a href=\"" << fileName << "\">";
        ret << std::setw(53) << std::setfill(' ');
        ret << std::left << (fileName + std::string("</a>"));
        ret << std::right;
        ret << folder.getFiles(i)->generateTime;
        ret << std::setw(20) << std::setfill(' ');
        if (folder.getFiles(i)->type == FileController::DIRECTORY) {
            ret << "-" << std::endl;
        } else {
            ret << folder.getFiles(i)->size << std::endl;
        }
    }
    ret << "</pre><hr></body>";
    ret << "</html>";
    return (ret.str());
}

std::string HTMLBody::getRedirectBody(const HTTPData& data) {
    std::stringstream ret;
    ret << "<head>";
    ret << "<meta http-equiv=\"refresh\" content=\"0;URL=";
    ret << "\'" << data._resAbsoluteFilePath << "\'\"";
    ret << " />";
    ret << "</head>";
    return (ret.str());
}

std::string HTMLBody::getStaticHTML(HTTPData& data) {
    std::string ret;

    std::string statusMsg;
    std::stringstream ssStatusCode;
    ssStatusCode << data._statusCode;
    if (data._statusCode == 200) {
        ret = getAutoIndexBody(data._root, data._URIFilePath);
    } else if (data.getResStartLineMap(data._statusCode).empty() == false) {
        statusMsg = ssStatusCode.str() + " " + data.getResStartLineMap(data._statusCode);
        ret = getBasicHTMLBody(statusMsg);
    } else {
        throw ErrorHandler("Error: invalid HTTP Status Code", ErrorHandler::ALERT, "HTMLBody::getStaticHTML");
    }
    return (ret);
}