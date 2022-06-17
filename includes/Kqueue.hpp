#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <iostream>
#include <map>
#include <vector>
#include <sys/event.h>

#define NOE 1000

class Client;

class Kqueue
{
private:
	int kq_fd;
	std::vector<struct kevent> change_list;
	struct kevent event_list[NOE];

public:
	Kqueue(/* args */);
	~Kqueue();

	const int &get_kq_fd(void) const;
	struct kevent *get_event_list(void);
	std::vector<struct kevent> &get_change_list(void);

	void set_change_list(std::vector<struct kevent> change_list_);
	void set_event_list(struct kevent &event_list_);
	void set_kq_fd(int fd_);
	int set_event(void);

	void setting(void);
};

#endif