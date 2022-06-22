#include "../includes/Client.hpp"

Client::Client(/* args */) : server_sock(-1), server_id(-1), location_id(-1), read_fd(-1), write_fd(-1), status(no), is_file(0), RETURN(0), pid(-1), read_fp(NULL), write_fp(NULL), chunked(-1) {}

Client::~Client()
{
	if (this->read_fp != NULL)
		fclose(this->read_fp);
	if (this->write_fp != NULL)
		fclose(this->write_fp);
}

void Client::header_parsing(std::string &read_str)
{
	std::string header;
	std::string temp = read_str;

	unsigned long find;
	if ((find = temp.find("X-Powered-By:")) != std::string::npos) // if cgi -> header parsing
	{
		if ((find = temp.find("<")) != std::string::npos)
		{
			header = temp.erase(find - 1, temp.end() - temp.begin());
			read_str = read_str.erase(0, find);
		}
	}
	// std::out << "header:" << header << std::endl;
	this->get_request().set_header(header);
}

int Client::request_parsing(FILE *file_ptr)
{
	char buff[BUFSIZE];
	memset(buff, 0, BUFSIZE);
	long valfread = 0;
	if ((valfread = fread(buff, sizeof(char), BUFSIZE - 1, file_ptr)) > 0)
	{
		buff[valfread] = 0;
		string_buff.append(buff, valfread);
	}
	else if (valfread < 0)
		return -1;
	else
		return 1;

	if (this->status == chunked_WAIT)
	{
		unsigned long find;
		if ((string_buff.find("Transfer-Encoding: chunked")) != std::string::npos)
		{
			this->chunked = 1;
			int hex_to_int;

			std::cerr << "this request is chunked!!\n";
			if ((find = string_buff.find("\r\n\r\n")) != std::string::npos)
			{
				if (this->chunked_header == "")
					this->chunked_header = string_buff.substr(0, find + 4);

				std::string body;
				body = string_buff.erase(0, find + 4);
				std::string::iterator it = body.begin();
				std::string size;
				// hex

				it = body.begin();
				while (it != body.end())
				{
					find = body.find("\r\n");
					size = body.substr(0, find);
					body.erase(0, find + 2);
					hex_to_int = strtol(size.c_str(), NULL, 16);
					size.clear();
					if (hex_to_int == 0) // 14
					{
						this->status = chunked_FINISH;
						break;
					}

					find = body.find("0\r\n"); // (abcdefg1234567)
					this->chunked_body += body.substr(0, find);
				}
			}
		}
		else if (valfread < BUFSIZE - 1) // not chunked & all read
		{
			this->status = chunked_FINISH;
		}
	}
	if (this->status == chunked_FINISH)
	{
		if (this->chunked == 1)
			this->request.split_request(this->chunked_header + this->chunked_body); // chunked
		else
			this->request.split_request(this->string_buff);
		this->string_buff.clear();
		this->request.request_parsing(this->request.get_requests());
		this->request.query_parsing();
		return 0;
	}
	else
	{
		return 1;
	}
}

const int &Client::get_server_sock(void) const { return this->server_sock; }
const int &Client::get_status(void) const { return this->status; }
const int &Client::get_read_fd(void) const { return this->read_fd; }
const int &Client::get_write_fd(void) const { return this->write_fd; }
const std::string &Client::get_route(void) const { return this->route; }
const int &Client::get_server_id(void) const { return this->server_id; }
const int &Client::get_location_id(void) const { return this->location_id; }
const int &Client::get_is_file(void) const { return this->is_file; }
const int &Client::get_RETURN(void) const { return this->RETURN; }
const int &Client::get_pid(void) const { return this->pid; }
Request &Client::get_request(void) { return this->request; }
Response &Client::get_response(void) { return this->response; }
const std::string &Client::get_content_type(void) const { return this->content_type; }
const std::string &Client::get_open_file_name(void) const { return this->open_file_name; }
const FILE *Client::get_read_fp(void) const { return this->read_fp; }
const FILE *Client::get_write_fp(void) const { return this->write_fp; }
const char *Client::get_ip(void) const { return this->ip; }
const std::string &Client::get_redi_root(void) const { return this->redi_root; }


void Client::set_server_sock(int fd) { this->server_sock = fd; }
void Client::set_status(int ok)
{
	this->status = ok;
	switch (ok)
	{
	case is_file_read_ok:
		break;

	default:
		break;
	}
}
void Client::set_read_fd(int fd) { this->read_fd = fd; }
void Client::set_write_fd(int fd) { this->write_fd = fd; }
// void Client::set_server_it(std::vector<Server>::iterator server_it_) { this->server_it = server_it_; }
void Client::set_route(std::string str) { this->route = str; }
void Client::set_server_id(int i) { this->server_id = i; }
void Client::set_location_id(int i) { this->location_id = i; }
void Client::set_is_file(int i) { this->is_file = i; }
void Client::set_RETURN(int i) { this->RETURN = i; }
void Client::set_request(Request request_) { this->request = request_; }
void Client::set_response(Response response_) { this->response = response_; }
void Client::set_content_type(std::string str) { this->content_type = str; }
void Client::set_open_file_name(std::string str) { this->open_file_name = str; }
void Client::set_read_fp(FILE *_fp) { this->read_fp = _fp; }
void Client::set_write_fp(FILE *_fp) { this->write_fp = _fp; }
void Client::set_pid(int i) { this->pid = i; }
void Client::set_redi_root(std::string str) { this->redi_root = str; }
