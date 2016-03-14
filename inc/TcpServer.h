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
	bool SetupSignalHandlers();
	bool StartProcessThread(int connfd);

	static void * ProcessRequestThread(void *arg);
	static void sig_chld(int signo);

	int listenfd;
};

#endif
