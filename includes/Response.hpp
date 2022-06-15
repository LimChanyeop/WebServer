#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <vector>

class Response
{
public:
	std::string response_str;	  // temp res
	std::string send_to_response; // real res
	int response_lenth;

public:
	Response(/* args */);
	~Response();

	void set_header(const int &status, const std::string &route);
	void set_autoindex(std::string &referer, const std::string &name, const int &i);
	std::string &get_send_to_response(void);
	void clear_response(void);
};

#endif