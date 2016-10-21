#ifndef SOCKSSERVER_H
#define SOCKSSERVER_H

#include <string>
#include <signal.h>
#include "proxy.h"

class TcpServer
{
public:
	void Run(std::string srvAddr, int srvPort);

private:
	int CreateListenSocket(std::string srvAddr, int srvPort);
	bool StartProcessThread(int connfd);
	static void* ProcessRequestThread(void *arg);
};

#endif
