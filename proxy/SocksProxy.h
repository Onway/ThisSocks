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
public:
    SocksServerProxy();
    void Run(int srcfd);

protected:
    void Run(int srcfd, const char *request, int len);

private:
    bool SelectMethod();
    bool ValidateSource();
    int ConnectServer();

    Passwd pwd;
};

class SocksClientProxy : public SocksProxy
{
public:
    void Run(int connfd);

protected:
    int ConnectServer();
    void Run(int srcfd, const char *request, int len);

private:
    bool WaitingMethod(int srcfd);
    bool RequestProxy();
    bool ResponseMethod(int srcfd);
};

#endif // SOCKSPROXY_H
