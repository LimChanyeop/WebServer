#include "ServerSocket.hpp"

void ServerSocket::set_sockaddr() {
    address = new sockaddr_in;
    memset((char *)address, 0, sizeof(*address));
    address->sin_family = AF_INET; // tcp
    address->sin_port = htons(PORT);
    address->sin_addr.s_addr = htonl(INADDR_ANY); // open inbound restriction
    memset(address->sin_zero, 0, sizeof(address->sin_zero));
    address_len = sizeof(*address);
}

void ServerSocket::set_address_len(unsigned int len) { address_len = len; }

const sockaddr_in *ServerSocket::get_address(void) const { return (address); }

unsigned int &ServerSocket::get_address_len(void) { return address_len; }
