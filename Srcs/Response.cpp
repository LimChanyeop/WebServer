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

void Response::set_autoindex(const std::string &referer, const std::string &name, const int &i) // opendir != NULL, readdir, closedir
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

void Response::set_header(const int &status, const std::string &opt, const std::string &content_type)
{
	if (status == 200)
	{
		std::cout << "200 OK\n";
		if (opt != "") // cgi
		{
			this->send_to_response = "HTTP/1.1 200 OK\r\n";
			this->send_to_response += opt;
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
		// std::cout << this->response_str;
	}
	else if (status == 201)
	{
		std::cout << "201 Created\n";
		this->send_to_response = "HTTP/1.1 201 Created\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
	else if (status == 204) // POST
	{
		std::cout << "204 No Content\n";
		this->send_to_response = "HTTP/1.1 204 No Content\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
	else if (status == 301)
	{
		std::cout << "301 Moved Permanently\n";
		this->send_to_response = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
		this->send_to_response += opt + "\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
	}
	else if (status == 400)
	{
		std::cout << "400 Bad Request\n";
		this->send_to_response = "HTTP/1.1 400 Bad Request\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
	else if (status == 405)
	{
		std::cout << "405 Method Not Allowed\n";
		this->send_to_response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
	else if (status == 413)
	{
		std::cout << "413 Request Entity Too Large\n";
		this->send_to_response = "HTTP/1.1 413 Request Entity Too Large\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
	else if (status == 500)
	{
		std::cout << "500 INTERNAL_SERVER_ERROR\n";
		this->send_to_response = "HTTP/1.1 500 INTERNAL_SERVER_ERROR\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
	else // 404
	{
		std::cout << "404 Not Found\n";
		this->send_to_response = "HTTP/1.1 404 Not Found\r\nContent-Type: ";
		this->send_to_response += content_type;
		this->send_to_response += "\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
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

void Response::set_response_str(const std::string &str)
{
	this->response_str = str;
}

void Response::set_response_length(const int &length)
{
	this->response_length = length;
}

std::string &Response::get_response_str(void)
{
	return this->response_str;
}

int &Response::get_response_lenth(void)
{
	return this->response_length;
}