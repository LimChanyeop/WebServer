#include "../includes/Fd.hpp"

Fd::Fd(/* args */)// : fd(0), socket_type(0), address_len(0)
{

}

Fd::~Fd()
{
}

void Fd::set_socket_fd(int fd_)
{
	fd = fd_;
}

const int &Fd::get_socket_fd(void) const
{
	return fd;
}

void Fd::set_address_len(unsigned int len)
{
	this->address_len = len;
}

sockaddr_in &Fd::get_address(void)
{
	return this->address;
}

unsigned int &Fd::get_address_len(void)
{
	return this->address_len;
}