#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <vector>

#define GET "GET"
#define POST "POST"
#define DELETE "DELETE"

enum RQ
{
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

class Request
{
public:
	int i;
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

	std::string response;

public:
	Request();
	void request_parsing(std::vector<std::string> &lists);
	int find_key(std::string key);
	void split_request(std::string lines);

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

	std::string get_method();
	std::string get_protocol();
	std::string get_host();
	std::string get_connection();
	std::string get_upgradeInSecureRequest();
	std::string get_userAgent();
	std::string get_accept();
	std::string get_acceptEncoding();
	std::string get_acceptLanguage();
	std::string get_cookie();
	std::string get_referer();
	std::string get_contentLength();
	std::string get_contentType();

	void print_request();
	void clear_request();
};

#endif