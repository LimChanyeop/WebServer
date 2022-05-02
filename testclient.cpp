#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <memory>
#include <iostream>

#define PORT 4242

void set_sockaddr(sockaddr_in *address)
{
	memset((char *)address, 0, sizeof(*address)); // ?
	address->sin_family = AF_INET;
	address->sin_port = htons(PORT);
}

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		std::cerr << "Please input message\n";
		exit(0);
	}
	sockaddr_in serv_add;
	char buffer[1024] = {0};

	int sock = socket(AF_INET, SOCK_STREAM, 0);

	set_sockaddr(&serv_add);
	int address_len = sizeof(serv_add);

	if (inet_pton(AF_INET, "127.0.0.1", &serv_add.sin_addr) <= 0)
	{
		std::cerr << "Invalid address or Address not supported\n";
		return -1;
	}

	if (connect(sock, (sockaddr *)&serv_add, address_len) < 0)
	{
		std::cerr << "Not connected!" << std::endl;
		return -1;
	}

	std::string bo = argv[1];

	send(sock, bo.c_str(), bo.length(), 0);
	std::cout << "message '" << bo << "' send!" << std::endl;

	int valread = read(sock, buffer, 1024);
	std::cout << "read : " << buffer << std::endl;

	return 0;
}
