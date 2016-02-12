#ifndef SOCKSSERVER_H
#define SOCKSSERVER_H

#include <string>
#include "Proxy.h"

class SocksServer
{
public:
	bool Init(std::string srvAddr, int srvPort);
	void Run(Proxy *proxy);

	int CreateListenSocket(std::string srvAddr, int srvPort);
	bool SetupSignalHandlers();

private:
	static void sig_chld(int signo);

	int listenfd;
};

#endif
