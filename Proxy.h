#ifndef PROXY_H
#define PROXY_H
#include "Passwd.h"
#include "Config.h"
#include "Logger.h"
#include "Utils.h"
#include "Encrypt.h"
#include <unistd.h>
#include <math.h>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

std::string GetSocketPair(int connfd);
class Proxy
{
public:
	virtual void Run(int connfd) = 0; 

protected:
    void ForwardData(int srcfd, int tarfd);
    int ReadN(int fd, void *buf, size_t count);

    EncryptBase *encrypter;

private:
    int ForwardData(int srcfd, int tarfd, bool fromClient);
};

class SocksServerProxy : public Proxy
{
public:
    SocksServerProxy();
    void Run(int srcfd);

private:
    bool SelectMethod();
    bool ValidateSource();
    int ConnectServer();

	Passwd pwd;
};

class SocksClientProxy : public Proxy
{
public:
	void Run(int connfd);

protected:
    int ConnectServer();

private:
    bool WaitingMethod(int srcfd);
    bool RequestProxy();
    bool ResponseMethod(int srcfd);
};

class HttpsClientProxy : public SocksClientProxy
{
public:
    void Run(int srcfd);
};

class HttpsServerProxy : public Proxy
{
public:
    void Run(int srcfd);
private:
    bool ParseIpPort(std::string &domain, uint32_t &ip, uint16_t &port);
};

class HttpClientProxy : public HttpsClientProxy
{
};

class HttpServerProxy : public Proxy
{
public:
    void Run(int srcfd);
private:
    bool ParseIpPort(std::string &domain, uint32_t &ip, uint16_t &port);
};
#endif
