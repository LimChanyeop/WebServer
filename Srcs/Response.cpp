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

void Response::set_autoindex(std::string root) // opendir != NULL, readdir, closedir 
{
	// std::vector<std::string>::iterator it = root.begin();
	// for (; it != root.end(); it++)
	// {
		this->response_str += "<!DOCTYPE html>\n";
		this->response_str += "<html>\n";
		this->response_str += "<head>\n</head>\n";
		this->response_str += "<body>\n";
		this->response_str += "<h1> Index of " + root + "</h1>\n";
		this->response_str += "<a href=" + root + ">";
		this->response_str += root;
		this->response_str += "</a><br>\n";
	// }
}

void Response::set_header(int i, std::string str_buf, std::string route)
{
	if (i == 200)
	{
		std::cout << "GET 200!!\n";
		this->send_to_response = "HTTP/1.1 200 OK\r\nContent-Type: "
					   "text/html\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
		// std::cout << this->response_str;
	}
	else if (i == 201) // POST
	{
		std::cout << "POST 201!!\n";
		// DIR	*dir_ptr = NULL;
		// struct dirent *file = NULL;
		// std::cout << route << std::endl;
		// if ((dir_ptr = opendir(route.c_str())) != NULL)
		// {
		// 	std::cerr << "It is directory, 400\n";
		// }
		// while((file = readdir(dir_ptr)) != NULL)
		// {
		// 	std::cout << file->d_name << std::endl;
		// }

		// std::ofstream ofs(route);
		// if (ofs.is_open() != 1)
		// {
		// 	std::cerr << "not opened";
		// 	exit(-1);
		// }
		// ofs.write("hi", 2);
		// ofs.close();
		
		this->send_to_response = "HTTP/1.1 201 Created\r\nContent-Type: "
					   "text/html\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
	else if (i == 204) // POST
	{
		std::cout << "POST 204!!\n";
		this->send_to_response = "HTTP/1.1 204 No Content\r\nContent-Type: "
					   "text/html\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
	}
	else if (i == 404)
	{
		std::cout << "404!!\n";
		this->send_to_response = "HTTP/1.1 404 Not Found\r\nContent-Type: "
					   "text/html\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
		// std::cout << this->response_str;
	}
	else if (i == 411)
	{
		std::cout << "POST 411!!\n";
		this->send_to_response = "HTTP/1.1 411 Not Found\r\nContent-Type: "
					   "text/html\r\nContent-Length: ";
		this->send_to_response += std::to_string(this->response_str.length() + 1);
		this->send_to_response += "\r\n\r\n";
		this->send_to_response += this->response_str + "\r\n";
		// std::cout << this->response_str;
	}
	else if (i == 42)
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
