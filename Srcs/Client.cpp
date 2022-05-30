#include "../includes/Client.hpp"

Client::Client(/* args */) : status(0)
{
}

Client::~Client()
{
}

const int &Client::get_server_sock(void) const
{
	return this->server_sock;
}
	const int &Client::get_status(void) const
	{
		return this->status;
	}

void Client::set_server_sock(int fd)
{
	this->server_sock = fd;
}
	void Client::set_status(int ok)
	{
		this->status = ok;
	}
