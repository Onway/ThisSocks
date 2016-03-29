#ifndef SOCKSPROXY_H
#define SOCKSPROXY_H

#include "ServerProxy.h"

class SocksServerProxy : public ServerProxy
{
public:
    static bool isMatch(const char *request, int);

protected:
    void Run(int srcfd, const char *request, int len, int &srvfd);
	void Process(int srcfd, const char *request, int len) const;
};

#endif // SOCKSPROXY_H
