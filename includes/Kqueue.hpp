#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <iostream>
#include <map>
#include <vector>
#include <sys/event.h>

#define NOE 100

class Client;

class Kqueue
{
public:
	int kq;
	std::vector<struct kevent> change_list;
	struct kevent event_list[NOE];

public:
	Kqueue(/* args */);
	~Kqueue();

	void setting(void);
	struct kevent *get_event_list(void);
	void set_kq_fd(int fd_);
	const int &get_kq_fd(void) const;
	int set_event(void);
};

#endif