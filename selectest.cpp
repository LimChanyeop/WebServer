#include <sys/select.h>
#include <iostream>
#include <unistd.h>
#include <time.h>

int main()
{
	// int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
	// FD_SET(fd, &fdset);
	// FD_CLR(fd, &fdset);
	// FD_ISSET(fd, &fdset);
	// FD_ZERO(&fdset);
	struct timeval tv = {/* tv_sec */10, /* tv_usec */0};
	fd_set fds;
	std::string c;

	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(1, &fds, NULL, NULL, &tv);
	if(FD_ISSET(STDIN_FILENO, &fds)) {
		std::cin >> c;
		std::cout << "Your input is '" << c << "'\n";
	}
	else
		puts("Timed out.");
}