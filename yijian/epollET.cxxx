#include "epollET.h"
namespace yijian {


socketTCPIP4::socketTCPIP4(uint16_t servPort, int listenQ) {
	sfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sfd_) {
		CKERROR(errno);
	}

	if (-1 == fcntl(sfd_, F_SETFL, O_NONBLOCK))
		CKERROR(errno);

	int opt = 1;
	setsockopt(sfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&servAddr_, sizeof(servAddr_));
	servAddr_.sin_family = AF_INET;
	servAddr_.sin_port = htons(servPort);
	servAddr_.sin_addr.s_addr = htonl(INADDR_ANY);
	if (int eno = bind(sfd_, (struct sockaddr *)&servAddr_, sizeof(struct sockaddr_in))) {
		CKERROR(eno);
	}
	if (int eno = listen(sfd_, listenQ)) {
		CKERROR(eno);
	}
}

epoll::epoll(int maxEvent,uint16_t servPort, int listenQ)
	 :maxEvent_(maxEvent),socket_(socketTCPIP4(servPort, listenQ)) {
	listenFd = socket_.listenSfd();
	epollfd_ = epoll_create(0);
	if (-1 == epollfd_) {
		CKERROR(errno);
	}
	auto pData = new data();
	pData->fd = listenFd;
	ev_.data.ptr = pData;
	ev_.events = EPOLLIN;
	if (-1 == epoll_ctl(epollfd_, EPOLL_CTL_ADD, pData->fd, &ev_)) {
		CKERROR(errno);
	}
}

void epoll::wait(ReadFunc& readf, WriteFunc& writef) {
	socklen_t clilen = sizeof(struct sockaddr_in);
	for(;;) {
		int nfds = epoll_wait(epollfd_, events_, MAX_EVENTS, -1);
		if (unlikely(-1 == nfds)) {
			CKERROR(errno);
		}
		for(int n = 0; n < nfds; ++n) {
			if (static_cast<data*>(events_[n].data.ptr)->fd == listenFd) {
				auto pData = new data();
				int connSfd = accept4(listenFd, (struct sockaddr *)(&pData->clientAddr), 
						&clilen, SOCK_NONBLOCK);
				pData->fd = connSfd;
				if (-1 == connSfd) 
					CKERROR(errno);
				ev_.events = EPOLLIN | EPOLLOUT | EPOLLET;
				ev_.data.ptr = pData;
			} else {
				auto p = static_cast<data*>(events_[n].data.ptr);
				if (events_[n].events & EPOLLIN) { //read
					p->isReadable = true;
					readf(p);
				}
				if (events_[n].events & EPOLLOUT) { //write
					p->isWritable = true;
					writef(p);
				}
			}
		}
	}
}


}
