// author jiwei.wang

#ifndef __YIJIAN_EPOLLET_H__
#define __YIJIAN_EPOLLET_H__

#include "macro.h"
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

#define MAX_EVENTS 10

namespace yijian {

class socketTCPIP4:public noncopyable {
public:
	socketTCPIP4(uint16_t servPort, int listenQ);
	int listenSfd() {
		return sfd_;
	}
private:
	int sfd_;
	struct sockaddr_in servAddr_;
};

class epoll:public noncopyable {

class data {
public:
	bool isReadable = false;
	bool isWritable = false;
	int fd;
	struct sockaddr_in clientAddr;
	std::shared_ptr<std::vector<char>> header;
	std::shared_ptr<std::vector<char>> body;
};

public:
	typedef std::function<void(data*)> ReadFunc;
	typedef std::function<void(data*)> WriteFunc;
	epoll(int maxEvent,uint16_t servPort, int listenQ);
	void wait(ReadFunc& readf, WriteFunc& writef);
	virtual ~epoll(){
		
	}
	
private:
	int maxEvent_;
	socketTCPIP4 socket_;
	int listenFd;
	int epollfd_;
	struct epoll_event ev_;
	struct epoll_event events_[MAX_EVENTS];
};


}

#endif
