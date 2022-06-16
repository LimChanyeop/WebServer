#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <vector>

#define GET "GET"
#define POST "POST"
#define DELETE "DELETE"

enum RQ {
    Emethod,
    Eprotocol,
    Ehost,
    Econnection,
    EupgradeInSecureRequest,
    EuserAgent,
    Eaccept,
    EacceptEncoding,
    EacceptLanguage,
    Ecookie,
    Ereferer,
    EcontentLength,
    EcontentType
};

class Request {
  public:
    std::vector<std::string> requests;
    std::string start_line;
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
    std::string post_body;
    std::string query;

  public:
    Request();
    void request_parsing(std::vector<std::string> &lists);
    int find_key(const std::string &key);
    void split_request(const std::string &lines);

    void set_method(std::string method);
    void set_protocol(std::string protocol);
    void set_host(std::string host);
    void set_connection(std::string connection);
    void set_upgradeInSecureRequest(std::string upgradeInSecureRequest);
    void set_userAgent(std::string userAgent);
    void set_accept(std::string accept);
    void set_acceptEncoding(std::string acceptEncoding);
    void set_acceptLanguage(std::string acceptLanguage);
    void set_cookie(std::string cookie);
    void set_referer(std::string referer);
    void set_contentLength(std::string contentLength);
    void set_contentType(std::string contentType);
    void set_query(std::string query);

    const std::string &get_method() const;
    const std::string &get_protocol() const;
    const std::string &get_host() const;
    const std::string &get_connection() const;
    const std::string &get_upgradeInSecureRequest() const;
    const std::string &get_userAgent() const;
    const std::string &get_accept() const;
    const std::string &get_acceptEncoding() const;
    const std::string &get_acceptLanguage() const;
    const std::string &get_cookie() const;
    const std::string &get_referer() const;
    const std::string &get_contentLength() const;
    const std::string &get_contentType() const;
    const std::string &get_query() const;

    void clear_request();
};

#endif