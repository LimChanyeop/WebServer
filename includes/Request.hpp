#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>

#define GET "get"
#define POST "post"
#define DELETE "delete"

class Request {
  private:
    std::string method;
    std::string protocol;
    std::string host;
    std::string connection;
    std::string upgradeInSecureRequest;
    std::string userAgent;
    std::string accept;
    std::string acceptEncoding;
    std::string acceptLanguage;
    std::string cookie;
    std::string referer;
    std::string contentLength;
    std::string contentType;

  private:
};

#endif