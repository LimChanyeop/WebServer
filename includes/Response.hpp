#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>

class Response
{
private:
	std::string response_str;

public:
	Response(/* args */);
	~Response();

	void set_response(int i, std::string str_buf);
	void clear_response(void);
	const std::string &get_response(void) const;
};

#endif