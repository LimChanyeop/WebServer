#include "../includes/Client.hpp"

Client::Client(/* args */) : status(no)
{
}

Client::~Client()
{
}

const int &Client::get_server_sock(void) const
{
	return this->server_sock;
}
const std::vector<Server>::iterator &Client::get_server_it(void) const
{
	return this->server_it;
}
	const int &Client::get_status(void) const
	{
		return this->status;
	}
const int &Client::get_read_fd(void) const
{
	return this->read_fd;
}


void Client::set_server_sock(int fd)
{
	this->server_sock = fd;
}
	void Client::set_status(int ok)
	{
		this->status = ok;
	}
void Client::set_read_fd(int fd)
{
	this->read_fd = fd;
}
void Client::set_server_it(std::vector<Server>::iterator server_it_)
{
	this->server_it = server_it_;
}
