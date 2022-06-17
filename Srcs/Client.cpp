#include "../includes/Client.hpp"

Client::Client(/* args */) : server_sock(-1), server_id(-1), location_id(-1), read_fd(-1), write_fd(-1), status(no), is_file(0), RETURN(0), pid(-1), fp(NULL) {}

Client::~Client()
{
	if (this->fp != NULL)
		fclose(this->fp);
} //////// 이거였어 해결!!!!

int Client::request_parsing(int event_ident)
{
	FILE *file_ptr = fdopen(event_ident, "r"); /////////////
	if (file_ptr == NULL)
	{
		std::cout << "id:" << event_ident << std::endl;
		std::cout << strerror(errno) << std::endl;
		return -1;
	}

	char buff[1024];
	memset(buff, 0, 1024);
	long fread_val = 0;
	std::string fread_str;
	// AddressSanitizer:DEADLYSIGNAL
	//  =================================================================
	//  ==38790==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000068 (pc 0x7fff6e9cca5d bp 0x7ffee81ed930 sp 0x7ffee81ed920 T0)
	//  ==38790==The signal is caused by a READ memory access.
	//  ==38790==Hint: address points to the zero page.
	//      #0 0x7fff6e9cca5d in flockfile+0x12 (libsystem_c.dylib:x86_64+0x38a5d)
	//      #1 0x7fff6e9ce6ac in fread+0x1e (libsystem_c.dylib:x86_64+0x3a6ac)
	while ((fread_val = fread(buff, sizeof(char), 1023, file_ptr)) > 0)
	{
		buff[fread_val] = 0;
		fread_str += buff;
	}
	if (fread_val < 0)
	{
		std::cerr << "request fread error\n";
	}
	this->fp = file_ptr;

	// std::cerr << "Cli::fread_str: " << fread_str << std::endl;
	this->request.split_request(fread_str);
	this->request.request_parsing(this->request.requests);
	return 1;
}

const int &Client::get_server_sock(void) const { return this->server_sock; }
const int &Client::get_status(void) const { return this->status; }
const int &Client::get_read_fd(void) const { return this->read_fd; }
const int &Client::get_write_fd(void) const { return this->write_fd; }
const std::string &Client::get_route(void) const { return this->route; }
const int &Client::get_server_id(void) const { return this->server_id; }
const int &Client::get_location_id(void) const { return this->location_id; }

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
