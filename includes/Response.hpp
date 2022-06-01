#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <vector>

class Response
{
private:
	std::string response_str;

public:
	Response(/* args */);
	~Response();

	void set_response(int i, std::string str_buf);
	void set_autoindex(std::string &str_buf, std::string root);
	std::string &get_response(void);
	void clear_response(void);
};

#endif