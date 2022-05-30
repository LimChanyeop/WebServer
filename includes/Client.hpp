#ifndef INFO_HPP
#define INFO_HPP

class Client
{
private:
	int server_sock;
	int status;
	
public:
	Client(/* args */);
	~Client();

	const int &get_server_sock(void) const;
	const int &get_status(void) const;

	void set_server_sock(int fd);
	void set_status(int ok);
};

#endif