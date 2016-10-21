#ifndef SERVERPROXY_H
#define SERVERPROXY_H

#include "proxy.h"

class ServerProxy : public Proxy
{
protected:
	void Process(int srcfd);
    int ConnectRealServer(uint32_t ip, uint16_t port) const;
	uint32_t GetIPv4ByName(std::string hostname) const;
	virtual void Process(int srcfd, const char *request, int len) const;

private:
    bool ValidateProxyClient() const;
	ServerProxy* SelectServerProxy(const char *request, int len) const;
};

#endif // SERVERPROXY_H
