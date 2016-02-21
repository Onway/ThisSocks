#ifndef CLIENTPROXY_H
#define CLIENTPROXY_H

#include "Proxy.h"

class ClientProxy : public Proxy
{
protected:
    void Run(int srcfd, const char *request, int len);
};

#endif // CLIENTPROXY_H
