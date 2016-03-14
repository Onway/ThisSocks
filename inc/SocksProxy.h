#ifndef SOCKSPROXY_H
#define SOCKSPROXY_H

#include "Proxy.h"

class SocksServerProxy : public Proxy
{
public:
    static bool isMatch(const char *request, int);

protected:
    void Run(int srcfd, const char *request, int len, int &srvfd);
};

#endif // SOCKSPROXY_H
