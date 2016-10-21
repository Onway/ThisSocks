#ifndef HTTPSPROXY_H
#define HTTPSPROXY_H

#include "server_proxy.h"

class HttpsServerProxy : public ServerProxy
{
public:
    static bool isMatch(const char *request, int len);

protected:
	void Process(int srcfd, const char *request, int len) const;

private:
    bool ParseIpPort(std::string &domain, uint32_t &ip, uint16_t &port) const;
};

#endif // HTTPSPROXY_H
