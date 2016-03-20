#ifndef SOCKSSERVER_H
#define SOCKSSERVER_H

#include <string>
#include <signal.h>
#include "Proxy.h"

class TcpServer
{
public:
	bool Init(std::string srvAddr, int srvPort);
	void Run();

private:
	int CreateListenSocket(std::string srvAddr, int srvPort);
	bool StartProcessThread(int connfd);
	static void * ProcessRequestThread(void *arg);

	int listenfd;
};

#endif
