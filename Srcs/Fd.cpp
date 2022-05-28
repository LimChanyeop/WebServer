#include "../includes/Fd.hpp"

Fd::Fd(/* args */)
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

void Fd::set_kq_fd(int fd_)
{
	fd = fd_;
}

const int &Fd::get_kq_fd(void) const
{
	return fd;
}
