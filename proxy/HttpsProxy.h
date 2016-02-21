#ifndef HTTPSPROXY_H
#define HTTPSPROXY_H

#include "Proxy.h"

class HttpsProxy : public Proxy
{
public:
    bool isMatch(const char *request, int len);
};

class HttpsClientProxy : public HttpsProxy
{
public:
    void Run(int srcfd);
protected:
    void Run(int srcfd, const char *request, int len);
};

class HttpsServerProxy : public HttpsProxy
{
public:
    void Run(int srcfd);

protected:
    void Run(int srcfd, const char *request, int len);

private:
    bool ParseIpPort(std::string &domain, uint32_t &ip, uint16_t &port);
};

#endif // HTTPSPROXY_H
