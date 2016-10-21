#ifndef SOCKSPROXY_H
#define SOCKSPROXY_H

#include "server_proxy.h"

class SocksServerProxy : public ServerProxy
{
public:
    static bool isMatch(const char *request, int);

protected:
	void Process(int srcfd, const char *request, int len) const;
};

#endif // SOCKSPROXY_H
