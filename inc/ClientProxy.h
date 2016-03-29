#ifndef CLIENTPROXY_H
#define CLIENTPROXY_H

#include "Proxy.h"

class ClientProxy : public Proxy
{
protected:
    void Run(int srcfd, const char *request, int len, int &srvfd);
	void Process(int srcfd);

private:
    int ConnectProxyServer() const;
    bool LoginProxyServer() const;
};

#endif // CLIENTPROXY_H
