#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include "Socket.hpp"

class ServerSocket : public Socket
{
private:
  sockaddr_in address;
  unsigned int address_len;

public:
  ServerSocket();
  void set_sockaddr();
  sockaddr_in &get_address(void);
  void set_address_len(unsigned int len);
  unsigned int &get_address_len(void);
};

#endif