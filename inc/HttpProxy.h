#ifndef HTTPPROXY_H
#define HTTPPROXY_H

#include "ServerProxy.h"

class HttpServerProxy : public ServerProxy
{
public:
    static bool isMatch(const char *request, int len);

protected:
    void Run(int srcfd, const char *request, int len, int &srvfd);

private:
    bool ParseIpPort(std::string &domain, uint32_t &ip, uint16_t &port);
};

#endif // HTTPPROXY_H
