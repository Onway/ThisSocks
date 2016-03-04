#ifndef HTTPSPROXY_H
#define HTTPSPROXY_H

#include "Proxy.h"

class HttpsServerProxy : public Proxy
{
public:
    static bool isMatch(const char *request, int len);

protected:
    void Run(int srcfd, const char *request, int len);

private:
    bool ParseIpPort(std::string &domain, uint32_t &ip, uint16_t &port);
};

#endif // HTTPSPROXY_H
