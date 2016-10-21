#ifndef CLIENTPROXY_H
#define CLIENTPROXY_H

#include "proxy.h"

class ClientProxy : public Proxy
{
protected:
	void Process(int srcfd);

private:
    int ConnectProxyServer() const;
    bool LoginProxyServer() const;
};

#endif // CLIENTPROXY_H
