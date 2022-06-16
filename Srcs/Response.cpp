#include "../includes/Response.hpp"
#include <dirent.h>
#include <iostream>
#include <fstream>

Response::Response(/* args */)
{
}

Response::~Response()
{
}

void Response::set_autoindex(std::string &referer, const std::string &name, const int &i) // opendir != NULL, readdir, closedir
{
	// /(View) + /redi.html
	std::string route;
	if (name == ".." && i == 1)
		return;
	else if (*name.begin() == '.' && i == 1)
	{
		if (*(name.end() - 1) != '/')
			route = name + '/'; ///
		else
			route = name;
	}
	else if (i == 0)
	{
		if (*(referer.end() - 1) != '/')
			route = referer + '/' + name;
		else
			route = referer + name;
	}
	else
		route = '/' + name;
	this->response_str += "<head>\n</head>\n";
	this->response_str += "<body>\n";
	this->response_str += "<a href=" + route + ">";
	this->response_str += name + " [route:" + route + ']';
	this->response_str += "</a><br>\n";
}

void Response::set_header(const int &status, const std::string &header, const std::string &content_type)
{
	if (status == 200)
	{
		std::cout << "GET 200!!\n";
		if (header != "") // cgi
		{
			this->send_to_response = "HTTP/1.1 200 OK\r\n";
			std::cerr << "there is header!!\n";
			this->send_to_response += header;
			this->send_to_response += "Content-Length: ";
		}
		else
		{
			this->send_to_response = "HTTP/1.1 200 OK\r\nContent-Type: ";
			this->send_to_response += content_type;
			this->send_to_response += "\r\nContent-Length: ";
		}
		this->send_to_response += std::to_string(this->response_str.length() + 2);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
		// std::cerr << "Res::response+str:" << response_str << std::endl;
		// std::cout << this->response_str;
	}
	else if (status == 201)
	{
		std::cout << "POST 201!!\n";\
		this->send_to_response = "HTTP/1.1 201 Created\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
	else if (status == 204) // POST
	{
		std::cout << "POST 204!!\n";
		this->send_to_response = "HTTP/1.1 204 No Content\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
	else if (status == 404)
	{
		std::cout << "404!!\n";
		this->send_to_response = "HTTP/1.1 404 Not Found\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
		// std::cout << this->response_str;
	}
	else if (status == 411)
	{
		std::cout << "POST 411!!\n";
		this->send_to_response = "HTTP/1.1 411 Not Found\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
		// std::cout << this->response_str;
	}
	else if (status == 42)
	{
		this->send_to_response = "HTTP/1.1 200 OK\r\n";
		this->send_to_response += "cache-control: max-age=31536000\r\n";
		this->send_to_response += "content-encoding: gzip\r\n";
		this->send_to_response += "content-length: ";
		this->send_to_response += std::to_string(this->response_str.size());
		this->send_to_response += "\r\ncontent-type: image/vnd.microsoft.icon\r\n";
		this->send_to_response += "referrer-policy: unsafe-url";
		this->send_to_response += "server: NWS\r\n";
		this->send_to_response += "strict-transport-security: max-age=63072000; includeSubdomains\r\n";
		this->send_to_response += "vary: Accept-Encoding,User-Agent\r\n";

		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
}

void Response::clear_response(void)
{
	response_str.clear();
}

std::string &Response::get_send_to_response(void)
{
	return this->send_to_response;
}
