#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <vector>

class Response
{
private:
	std::string response_str;	  // temp res
	std::string send_to_response; // real res
	int response_length;

public:
	Response(/* args */);
	~Response();

	std::string &get_send_to_response(void);
	std::string &get_response_str(void);
	int &get_response_lenth(void);

	void set_header(const int &status, const std::string &route, const std::string &content_type);
	void set_autoindex(const std::string &referer, const std::string &name, const int &i);
	void set_response_str(const std::string &str);
	void set_response_length(const int &length);

	void clear_response(void);
};

#endif