#include "../includes/Request.hpp"

Request::Request() : referer("/"), contentType("text/plain"), post_body_size(0), header_size(0) {}

void Request::request_parsing(const std::vector<std::string> &lists)
{
	std::vector<std::string> lists_ = lists;
	for (std::vector<std::string>::iterator it = lists_.begin(); it != lists_.end(); it++)
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
			// std::cout << *it << std::endl;
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
		case EcontentType:
			contentType = *(++it);
			break;
		case 13: // GET
			set_method("GET");
			referer = *(++it);
			// std::cout << "referer:" << referer << std::endl;
			break;
		case 14: // POST
			set_method("POST");
			referer = *(++it);
			break;
		case 15: // DELETE
			set_method("DELETE");\
			referer = *(++it);
			break;
		default:
			// std::cerr << "Invalid input\n";
			break;
		}
	}
}

int Request::find_key(const std::string &key)
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
	keys.push_back("DELETE");
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

void Request::split_request(const std::string &lines)
{
	int idx;
	// if ((idx = lines.find("\r\n\r\n")) != std::string::npos) {
	//     std::string temp = lines;
	//     this->post_body = temp.erase(0, idx + 2);
	//     // std::cout << "Request::body : " << this->post_body << std::endl;
	// } else
	
	for (std::string::const_iterator it = lines.begin(); it != lines.end(); it++)
	{
		if (*it == '\n')
			break;
		this->start_line += *it;
		// std::cerr << this->post_filename << std::endl;
	}
	int find;
	std::string head = lines;
	if ((find = head.find("\r\n\r\n")) != std::string::npos)
	{
		this->post_body = head.substr(find + 4); // std::cout << "cant found body\n";
		this->post_body_size = this->post_body.size();
		// std::cerr << "post_body:" << this->post_body << ", size: " << post_body_size << std::endl;
		if ((find = post_body.find("filename=")) != std::string::npos)
		{
			std::cout << "find filename\n";
			std::string::iterator it = post_body.begin() + find;
			for (int i = 0; it != post_body.end(); it++)
			{
				if (i == 2)
					break;
				else if (*it == '\"')
					i++;
				this->post_filename += *it;
				// std::cerr << this->post_filename << std::endl;
			}
		}
		// std::cerr << this->post_body << std::endl;
		if ((find = post_body.find("Content-Type: ")) != std::string::npos)
		{
			std::cout << "find content type\n";
			std::string::iterator it = post_body.begin() + find;
			for (; it != post_body.end(); it++)
			{
				if (*it == '\n')
					break;
				this->post_content_type += *it;
			}
		}
		if ((find = post_body.find("------")) != std::string::npos) // boundary
		{
			std::cout << "find -------------\n";
			std::string::iterator it = post_body.begin() + find;
			for (; it != post_body.end(); it++)
			{
				if (*it == '\n')
					break;
				this->boundary += *it;
				this->post_header += *it;
			}
			this->boundary.erase(0, 2);
			std::cout << "boundary: " << boundary << std::endl;
			
			if ((find = post_body.find("\r\n\r\n", it - post_body.begin())) != std::string::npos) // it 부터 찾게
				post_header += post_body.substr(it - post_body.begin(), find - (it - post_body.begin()) + 4 + 8);
			// std::cerr << "start(it - begin): " << it - post_body.begin() << ", find: " << find << std::endl;
		}
	}
	if (find != -1) // post
		this->header_size = find;
	else
		this->header_size = head.size();

	
	// std::cout << "header: " << head << "\nheader_size: " << header_size << std::endl;
	// std::cerr << "Cli::body ======================================\n" << post_body << "\n===============================================\n";
	// std::cerr << "Cli::header ======================================\n" << post_header << "\n===============================================\n";
	
	if (post_header == "")
		post_header = post_body; ///////


	std::string delim = " \t\n";
	std::string::const_iterator it;
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


void Request::set_start_line(std::string _start_line) { this->start_line = _start_line; }
void Request::set_method(std::string method) { this->method = method; }
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
	// std::cout << "TEST-host:" << host << std::endl;
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
void Request::set_post_content_type(std::string contentLength) { this->post_content_type = contentLength; }
void Request::set_post_filename(std::string filename) { this->post_filename = filename; }
void Request::set_boundary(std::string boundary) { this->boundary = boundary; }
void Request::set_header(std::string str) { this->post_header = str; }
void Request::set_post_body_size(int i) { this->post_body_size = i; }
void Request::set_header_size(int i) { this->header_size = i; }

const std::string &Request::get_start_line() const { return start_line; }
const std::string &Request::get_method() const { return method; }
const std::string &Request::get_protocol() const { return protocol; }
const std::string &Request::get_host() const { return host; }
const std::string &Request::get_connection() const { return connection; }
const std::string &Request::get_upgradeInSecureRequest() const { return upgradeInSecureRequest; }
const std::string &Request::get_userAgent() const { return userAgent; }
const std::string &Request::get_accept() const { return accept; }
const std::string &Request::get_acceptEncoding() const { return acceptEncoding; }
const std::string &Request::get_acceptLanguage() const { return acceptLanguage; }
const std::string &Request::get_cookie() const { return cookie; }
const std::string &Request::get_referer() const { return referer; }
const std::string &Request::get_contentLength() const { return contentLength; }
const std::string &Request::get_contentType() const { return contentType; }
const std::string &Request::get_post_body() const { return post_body; }
const std::string &Request::get_post_content_type() const { return post_content_type; }
const std::string &Request::get_post_filename() const { return post_filename; }
const std::vector<std::string> &Request::get_requests() const { return requests; }
const std::string &Request::get_boundary(void) const {return this->boundary;}
const std::string &Request::get_header(void) const { return this->post_header; }
const int &Request::get_post_body_size(void) const { return this->post_body_size; }
const int &Request::get_header_size(void) const { return this->header_size; }


void Request::clear_request() { requests.erase(requests.begin(), requests.end()); }

const std::string &Request::get_query() const { return query; }

void Request::set_post_body(std::string post_body) { this->post_body = post_body; }