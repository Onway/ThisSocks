#ifndef PROXY_H
#define PROXY_H
#include <string>
#include "Passwd.h"
#include "Encrypt.h"

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

private:
    bool WaitingMethod(int srcfd);
    int ConnectServer();
    bool RequestProxy();
    bool ResponseMethod(int srcfd);
};

#endif
