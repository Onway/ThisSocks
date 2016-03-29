#ifndef SERVERPROXY_H
#define SERVERPROXY_H

#include "Proxy.h"

class ServerProxy : public Proxy
{
protected:
    void Run(int srcfd, const char *request, int len, int &srvfd);
	void Process(int srcfd);
    int ConnectRealServer(uint32_t ip, uint16_t port) const;

private:
    bool ValidateProxyClient() const;
	ServerProxy* SelectServerProxy(const char *request, int len) const;
};

#endif // SERVERPROXY_H
