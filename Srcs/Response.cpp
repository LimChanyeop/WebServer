#include "../includes/Response.hpp"

Response::Response(/* args */)
{
}

Response::~Response()
{
}

void Response::set_autoindex(std::string &str_buf, std::vector<std::string> root) // opendir != NULL, readdir, closedir 
{
	std::vector<std::string>::iterator it = root.begin();
	for (; it != root.end(); it++)
	{
		str_buf += "<!DOCTYPE html>\n";
		str_buf += "<html>\n";
		str_buf += "<head>\n</head>\n";
		str_buf += "<body>\n";
		str_buf += "<h1> Index of " + *it + "</h1>\n";
		str_buf += "<a href=" + *it + ">";
		str_buf += *it;
		str_buf += "</a><br>\n";
	}
}

void Response::set_response(int i, std::string str_buf)
{
	if (i == 1)
	{
		response_str = "HTTP/1.1 200 OK\r\nContent-Type: "
					   "text/html\r\nContent-Length: ";
		response_str += std::to_string(str_buf.length() + 1);
		response_str += "\r\n\r\n";
		response_str += str_buf + "\r\n";
		// std::cout << str_buf;
	}
	else if (i == 2)
	{
		response_str = str_buf;
	}
	else if (i == 42)
	{
		response_str = "HTTP/1.1 200 OK\r\n";
		response_str += "cache-control: max-age=31536000\r\n";
		response_str += "content-encoding: gzip\r\n";
		response_str += "content-length: ";
		response_str += std::to_string(str_buf.size());
		response_str += "\r\ncontent-type: image/vnd.microsoft.icon\r\n";
		response_str += "referrer-policy: unsafe-url";
		response_str += "server: NWS\r\n";
		response_str += "strict-transport-security: max-age=63072000; includeSubdomains\r\n";
		response_str += "vary: Accept-Encoding,User-Agent\r\n";
			
		response_str += "\r\n\r\n";
		response_str += str_buf + "\r\n";
	}
}

void Response::clear_response(void)
{
	response_str.clear();
}

std::string &Response::get_response(void)
{
	return response_str;
}
