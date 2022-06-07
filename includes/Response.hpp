#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <vector>

class Response
{
public:
	std::string response_str;
	std::string send_to_response;
	int response_lenth;

public:
	Response(/* args */);
	~Response();

	void set_header(int i, std::string str_buf, std::string route);
	void set_autoindex(std::string root);
	std::string &get_send_to_response(void);
	void clear_response(void);
};

#endif