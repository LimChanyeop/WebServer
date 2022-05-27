#include "../includes/Request.hpp"

Request::Request() : referer("/") {}

void Request::request_parsing(std::vector<std::string> &lists)
{
	this->i = 1;
	std::vector<std::string>::iterator it;
	for (it = lists.begin(); it != lists.end(); it++)
	{
		// std::cout << "it [" << *it << "] " << find_key(*it) << "\n\n";
		switch (find_key(*it))
		{
		case Emethod:
			method = *(++it);
			break;
		case Eprotocol:
			protocol = *(++it);
			break;
		case Ehost:
			this->set_host(*(++it));
			std::cout << *it << std::endl;
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
		case EcontentLength:
			contentLength = *(++it);
			break;
		case 13: // GET
			start_line = "GET";
			referer = *(++it);
			std::cout << "referer:" << referer << std::endl;
			break;
		case 14: // POST
			start_line = "POST";
			break;
		default:
			// std::cerr << "Invalid input\n";
			break;
		}
	}
}

int Request::find_key(std::string key)
{
	std::vector<std::string> keys;
	int result = 0;
	keys.push_back("Method:");
	keys.push_back("Protocol:");
	keys.push_back("Host:");
	keys.push_back("Connection:");
	keys.push_back("Upgrade-InSecure-Request:");
	keys.push_back("User-Agent:");
	keys.push_back("Accept:");
	keys.push_back("Accept-Encoding:");
	keys.push_back("Accept-Language:");
	keys.push_back("Cookie:");
	keys.push_back("Referer:");
	keys.push_back("Content-Length:");
	keys.push_back("Content-Type:");
	keys.push_back("GET");
	keys.push_back("POST");
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
	std::string delim = " \t\n";
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
// void Request::split_request(std::string lines)
// {
// 	std::string::iterator it = lines.begin();
// 	std::string attr = "";
// 	while (it != lines.end())
// 	{
// 		while (*it != ' ' && it != lines.end())
// 		{
// 			attr += *it;
// 			++it;
// 		}
// 		requests.push_back(attr);
// 		attr.clear();
// 		if (it != lines.end())
// 			++it;
// 		while (*it != '\n' && it != lines.end())
// 		{
// 			attr += *it;
// 			++it;
// 		}
// 		requests.push_back(attr);
// 		attr.clear();
// 		if (it != lines.end())
// 			++it;
// 	}
// }

void Request::set_method(std::string method) { Request::method = method; }
void Request::set_protocol(std::string protocol) { this->protocol = protocol; }
void Request::set_host(std::string host)
{
	std::string::iterator it = host.begin();
	while (it != host.end())
	{
		if (isnumber(*it))
		{
			host.erase(host.begin(), it);
			break;
		}
		it++;
	}
	this->host = host;
	std::cout << "TEST-host:" << host << std::endl;
}
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
void Request::set_response(std::string response) { this->response = response;}

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

void Request::clear_request()
{
	requests.clear();
}