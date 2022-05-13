#include "../includes/Socket.hpp"

void Socket::set_socket_fd(int socket_fd) { this->socket_fd = socket_fd; }

int Socket::get_socket_fd() { return this->socket_fd; }