#include "../includes/Client.hpp"

Client::Client(/* args */) : server_sock(-1), server_id(-1), location_id(-1), read_fd(-1), write_fd(-1), status(chunked_WAIT), is_file(0), RETURN(0), pid(-1), fp(NULL) {}

Client::~Client()
{
	if (this->fp != NULL)
		fclose(this->fp);
} //////// 이거였어 해결!!!!

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

int Client::request_parsing(int event_ident)
{
	FILE *file_ptr = fdopen(event_ident, "r"); /////////////
	if (file_ptr == NULL)
	{
		return -1;
	}

	while (this->status != chunked_FINISH)
	{
		char buff[1024];
		memset(buff, 0, 1024);
		long valfread = 0;
		if ((valfread = fread(buff, sizeof(char), 1023, file_ptr)) > 0)
		{
			std::cout << "(read)\n";
			buff[valfread] = 0;
			this->chunked_str.append(buff, valfread);
		}
		std::cout << "valfread: " << valfread << std::endl;
		if (chunked_str.find())

		if (valfread == 0)
		{
			std::cout << "finish!\n";
			this->status = chunked_FINISH;
			this->fp = file_ptr;

			this->request.split_request(chunked_str);
			this->request.request_parsing(this->request.get_requests());
			this->request.query_parsing();
		}
		else if (valfread < 0)
		{
			return -1;
		}
	}
	return 0;
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
const FILE *Client::get_fp(void) const { return this->fp; }
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
void Client::set_fp(FILE *_fp) { this->fp = _fp; }
void Client::set_pid(int i) { this->pid = i; }
void Client::set_redi_root(std::string str) { this->redi_root = str; }
