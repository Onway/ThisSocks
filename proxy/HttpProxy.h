#ifndef HTTPPROXY_H
#define HTTPPROXY_H

#include "Proxy.h"

class HttpProxy : public Proxy
{
public:
    bool isMatch(const char *request, int len);
};

class HttpClientProxy : public HttpProxy
{
protected:
    void Run(int srcfd, const char *request, int len);
};

class HttpServerProxy : public HttpProxy
{
public:
    void Run(int srcfd);

protected:
    void Run(int srcfd, const char *request, int len);

private:
    bool ParseIpPort(std::string &domain, uint32_t &ip, uint16_t &port);
};

#endif // HTTPPROXY_H
