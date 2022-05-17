#include "../includes/Request.hpp"

void Request::request_parsing(std::vector<std::string> &lists)
{
	std::vector<std::string>::iterator it;
	for (it = lists.begin(); it != lists.end(); it++)
	{
		std::cout << "(" << *it << ")" << find_key(*it) << std::endl;
		switch (find_key(*it))
		{
		case Emethod:
			method = *(++it);
			break;
		case Eprotocol:
			protocol = *(++it);
			break;
		case Ehost:
			host = *(++it);
			break;
		case Econnection:
			connection = *(++it);
			break;
		case EupgradeInSecureRequest:
			upgradeInSecureRequest = *(++it);
			break;
		case EuserAgent:
			userAgent = *(++it);
			break;
		case Eaccept:
			accept = *(++it);
			break;
		case EacceptEncoding:
			acceptEncoding = *(++it);
			break;
		case EacceptLanguage:
			acceptLanguage = *(++it);
			break;
		case Ecookie:
			cookie = *(++it);
			break;
		case Ereferer:
			referer = *(++it);
			break;
		case EcontentLength:
			contentLength = *(++it);
			break;
		case 12:
			break;
		case 13:
			break;
		case 14: // server
			break;
		default:
			std::cerr << "Invalid input\n";
			break;
		}
	}
}

int Request::find_key(std::string key)
{
	std::vector<std::string> keys;
	int result = 0;
	keys.push_back("method");
	keys.push_back("protocol");
	keys.push_back("host");
	keys.push_back("connection");
	keys.push_back("upgradeInSecureRequest");
	keys.push_back("userAgent");
	keys.push_back("accept");
	keys.push_back("acceptEncoding");
	keys.push_back("acceptLanguage");
	keys.push_back("cookie");
	keys.push_back("referer");
	keys.push_back("contentLength");
	keys.push_back("contentType");
	std::vector<std::string>::iterator it;
	for (it = keys.begin(); it != keys.end(); it++)
	{
		if (*it == key)
			return result;
		result++;
	}
	if (it == keys.end())
		return -1;
	return -42;
}

void Request::split_request(std::string lines)
{
	std::string delim = " \t;";
	std::string::iterator it;
	std::string attr = "";
	for (it = lines.begin(); it != lines.end(); it++)
	{
		if (delim.find(*it) == std::string::npos)
		{
			attr += *it;
		}
		else
		{
			if (attr != "")
			{
				requests.push_back(attr);
				attr.clear();
			}
		}
	}
	if (attr.length() > 0)
	{
		requests.push_back(attr);
	}
}

void Request::set_method(std::string method) { Request::method = method; }
void Request::set_protocol(std::string protocol) { this->protocol = protocol; }
void Request::set_host(std::string host) { this->host = host; }
void Request::set_connection(std::string connection) { this->connection = connection; }
void Request::set_upgradeInSecureRequest(std::string upgradeInSecureRequest) { this->upgradeInSecureRequest = upgradeInSecureRequest; }
void Request::set_userAgent(std::string userAgent) { this->userAgent = userAgent; }
void Request::set_accept(std::string accept) { this->accept = accept; }
void Request::set_acceptEncoding(std::string acceptEncoding) { this->acceptEncoding = acceptEncoding; }
void Request::set_acceptLanguage(std::string acceptLanguage) { this->acceptLanguage = acceptLanguage; }
void Request::set_cookie(std::string cookie) { this->cookie = cookie; }
void Request::set_referer(std::string referer) { this->referer = referer; }
void Request::set_contentLength(std::string contentLength) { this->contentLength = contentLength; }
void Request::set_contentType(std::string contentType) { this->contentType = contentType; }

std::string Request::get_method() { return method; }
std::string Request::get_protocol() { return protocol; }
std::string Request::get_host() { return host; }
std::string Request::get_connection() { return connection; }
std::string Request::get_upgradeInSecureRequest() { return upgradeInSecureRequest; }
std::string Request::get_userAgent() { return userAgent; }
std::string Request::get_accept() { return accept; }
std::string Request::get_acceptEncoding() { return acceptEncoding; }
std::string Request::get_acceptLanguage() { return acceptLanguage; }
std::string Request::get_cookie() { return cookie; }
std::string Request::get_referer() { return referer; }
std::string Request::get_contentLength() { return contentLength; }
std::string Request::get_contentType() { return contentType; }

void Request::print_request()
{
	std::cout << "method: " << method << std::endl;
	std::cout << "protocol: " << protocol << std::endl;
	std::cout << "host: " << host << std::endl;
	std::cout << "connection: " << connection << std::endl;
	std::cout << "upgradeInSecureRequest: " << upgradeInSecureRequest << std::endl;
	std::cout << "userAgent: " << userAgent << std::endl;
	std::cout << "accept: " << accept << std::endl;
	std::cout << "acceptEncoding: " << acceptEncoding << std::endl;
	std::cout << "acceptLanguage: " << acceptLanguage << std::endl;
	std::cout << "cookie: " << cookie << std::endl;
	std::cout << "referer: " << referer << std::endl;
	std::cout << "contentLength: " << contentLength << std::endl;
	std::cout << "contentType: " << contentType << std::endl;
}