#include "HTMLBody.hpp"

std::string HTMLBody::getBasicHTMLBody(const std::string& statusMsg) {
    std::string serverName = "webserv/0.0.1";
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

std::string HTMLBody::getStaticHTML(const HTTPData& data) {
    std::string statusMsg;
    std::string ret;
    if (data._statusCode == 404) {
        statusMsg = "404 Not Found";
        ret = getBasicHTMLBody(statusMsg);
    } else if (data._statusCode == 403) {
        statusMsg = "403 Forbidden";
        ret = getBasicHTMLBody(statusMsg);
    } else if (data._statusCode == 400) {
        statusMsg = "400 Forbidden";
        ret = getBasicHTMLBody(statusMsg);
    } else if (data._statusCode == 413) {
        statusMsg = "413 Request Entity Too Large";
        ret = getBasicHTMLBody(statusMsg);
    } else if (data._statusCode == 500) {
        statusMsg = "500 Internal Server Error";
        ret = getBasicHTMLBody(statusMsg);
    } else if (data._statusCode == 200) {
        ret = getAutoIndexBody(data._root, data._URIFilePath);
    } else if (data._statusCode == 301) {
        ret = getRedirectBody(data);
    } else if (data._statusCode == 302) {
        ret = getRedirectBody(data);
    } else {
        throw ErrorHandler("Error: invalid HTTP Status Code", ErrorHandler::ALERT, "HTMLBody::getStaticHTML");
    }
    return (ret);
}