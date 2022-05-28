#include "../includes/Response.hpp"

Response::Response(/* args */)
{
}

Response::~Response()
{
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

const std::string &Response::get_response(void) const
{
	return response_str;
}
