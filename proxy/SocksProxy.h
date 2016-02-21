#ifndef SOCKSPROXY_H
#define SOCKSPROXY_H

#include "Proxy.h"

class SocksProxy : public Proxy
{
protected:
    bool isMatch(const char *request, int len);
};

class SocksServerProxy : public SocksProxy
{
protected:
    void Run(int srcfd, const char *request, int len);
};

class SocksClientProxy : public SocksProxy
{
protected:
    void Run(int srcfd, const char *request, int len);
};

#endif // SOCKSPROXY_H
