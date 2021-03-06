#include "../includes/Kqueue.hpp"

Kqueue::Kqueue(/* args */)
{
}

Kqueue::~Kqueue()
{
}

void Kqueue::setting(void)
{
	this->kq_fd = kqueue();
}

void Kqueue::set_kq_fd(int fd_)
{
	this->kq_fd = fd_;
}

const int &Kqueue::get_kq_fd(void) const
{
	return this->kq_fd;
}

int Kqueue::set_event(void)
{
	int num_of_event;
	int n_changes = this->change_list.size(); // number of changes = 등록하고자 하는 이벤트 수
	int n_event_list = NOE;
	if ((num_of_event = kevent(this->get_kq_fd(), &this->change_list[0], n_changes, this->get_event_list(), n_event_list, NULL)) == -1)
	{
		std::cerr << strerror(errno) << std::endl;
		std::cerr << "kevent error\n";
		exit(EXIT_FAILURE);
	}
	change_list.clear(); // 등록 이벤트 목록 초기화
	return num_of_event;
}

struct kevent *Kqueue::get_event_list(void) { return event_list; }
std::vector<struct kevent> &Kqueue::get_change_list(void) { return change_list; }