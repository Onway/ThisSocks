#include "Proxy.h"
#include "SocksProxy.h"
#include "HttpsProxy.h"
#include "HttpProxy.h"
#include "ClientProxy.h"
#include "Config.h"
#include "Logger.h"
#include "Utils.h"
#include "Encrypt.h"

using namespace std;

Proxy::Proxy() : encrypter(NULL)
{
}

Proxy::Proxy(const Proxy& proxy) : encrypter(NULL)
{
	if (proxy.encrypter != NULL) {
		encrypter = proxy.encrypter->clone();
	}
}

Proxy::~Proxy()
{
	if (encrypter != NULL) {
		delete encrypter;
		encrypter = NULL;
	}
}

Proxy& Proxy::operator=(const Proxy& proxy)
{
	if (encrypter != NULL) {
		delete encrypter;
	}
	encrypter = proxy.encrypter->clone();
	return *this;
}

void Proxy::Run(int srcfd)
{
	Proxy* proxy = NULL;
	if (GConfig.RunAsClient) {
		proxy = new ClientProxy();
	} else {
		proxy = new ServerProxy();
	}
	proxy->Process(srcfd);
	delete proxy;
}

void Proxy::Process(int srcfd)
{
}

void Proxy::ForwardData(int srcfd, int tarfd) const
{
    bool halfClose = false;
	fd_set fdset;
	while (1) {
        FD_ZERO(&fdset);
        FD_SET(srcfd, &fdset);
        FD_SET(tarfd, &fdset);
		
        if (select(max(srcfd, tarfd) + 1, &fdset, NULL, NULL, NULL) < 0) {
			if (errno == EINTR) {
				continue;
			}
            GLogger.LogErr(LOG_ERR, "select error");
			break;
		}

        int ret;
        if (FD_ISSET(srcfd, &fdset)) {
            if ((ret = ForwardData(srcfd, tarfd, true)) < 0) { // error
                break;
            } else if (ret == 0 && halfClose) {
                break;
            } else if (ret == 0) {
                halfClose = true;
				shutdown(tarfd, SHUT_WR);
            }
		}

        if (FD_ISSET(tarfd, &fdset)) {
            if ((ret = ForwardData(tarfd, srcfd, false)) < 0) {
                break;
            } else if (ret == 0 && halfClose) {
                break;
            } else if (ret == 0) {
                halfClose = true;
				shutdown(srcfd, SHUT_WR);
            }
		}
	}
}

int Proxy::ForwardData(int srcfd, int tarfd, bool fromClient) const
{
    bool encrypted = (GConfig.RunAsClient && !fromClient)
        || (!GConfig.RunAsClient && fromClient);

    char buf[MAXBUF];
    int readcnt;
    if (encrypted) {
        readcnt = encrypter->Read(buf, MAXBUF);
    } else {
        readcnt = read(srcfd, buf, MAXBUF);
    }
    if (readcnt < 0) {
        GLogger.LogErr(LOG_NOTICE, "read forward data error from %s",
                fromClient ? "client" : "server");
        return -1;
    }
    else if (readcnt == 0) {
        return 0;
    }

    int writecnt;
    if (encrypted) {
        writecnt = write(tarfd, buf, readcnt);
    } else {
        writecnt = encrypter->Write(buf, readcnt);
    }
    if (writecnt != readcnt) {
        GLogger.LogErr(LOG_ERR, "write forward data to %s error",
				fromClient ? "server" : "client");
        return -1;
	}

    return 1;
}
