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

Proxy::~Proxy()
{
	if (encrypter != NULL) {
		delete encrypter;
		encrypter = NULL;
	}
}

void Proxy::Run(int srcfd, int &srvfd)
{
    char request[MAXBUF];
    int len;
    Proxy* proxy = NULL;
    encrypter = NULL;
    do {
        if (GConfig.RunAsClient) {
            if ((len = read(srcfd, request, sizeof(request))) < 0) {
                GLogger.LogErr(LOG_NOTICE, "read proxy request error");
                return;
            } else if (len == 0) {
                // occurs when using system proxy setting in ubuntu 14.04
                GLogger.LogMsg(LOG_DEBUG, "proxy request connection closed");
                return;
            }

            proxy = SelectLocalProxy(GConfig.RunAsClient, request, len);
            if (proxy == NULL) {
                break;
            }

            int serverfd = ConnectProxyServer();
            if (serverfd < 0) {
                break;
            }
			srvfd = serverfd;

            encrypter = GEncryptFactory.GetEncrypter();
            if (!encrypter->SetClientFd(serverfd)) {
                break;
            }

            if (!LoginProxyServer()) {
                break;
            }
        } else {
            encrypter = GEncryptFactory.GetEncrypter();
            if (!encrypter->SetServerFd(srcfd)) {
                break;
            }

            if (!ValidateProxyClient()) {
                break;
            }

            if ((len = encrypter->Read(request, sizeof(request))) < 0) {
                GLogger.LogErr(LOG_NOTICE, "read proxy request error");
                break;
            }

            proxy = SelectLocalProxy(GConfig.RunAsClient, request, len);
            if (proxy == NULL) {
                break;
            }
        }

        proxy->encrypter = encrypter;
		encrypter = NULL;
        proxy->Run(srcfd, request, len, srvfd);
    } while (false);

    if (encrypter != NULL) {
        delete encrypter;
    }
    if (proxy != NULL) {
        delete proxy;
    }
}

Proxy* Proxy::SelectLocalProxy(bool isClient, const char *request, int len)
{
    if (isClient) {
        return new ClientProxy();
    } else {
        if (SocksServerProxy::isMatch(request, len)) {
            return new SocksServerProxy();
        }
        if (HttpsServerProxy::isMatch(request, len)) {
            return new HttpsServerProxy();
        }
        return new HttpServerProxy();
    }
    return NULL;
}

int Proxy::ConnectProxyServer()
{
    int tarfd;
    struct sockaddr_in taraddr;

    if ((tarfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        GLogger.LogErr(LOG_ERR, "create socket to proxy server error");
        return -1;
    }

    memset(&taraddr, 0, sizeof(taraddr));
    taraddr.sin_family = AF_INET;
    taraddr.sin_port = htons(GConfig.ServerPort);
    if (-1 == inet_pton(AF_INET,
            GConfig.ServerAddress.c_str(), &taraddr.sin_addr)) {
        GLogger.LogErr(LOG_ERR, "inet_pton error");
        return -1;
    }

    if (-1 == connect(tarfd,
            (struct sockaddr *)&taraddr, sizeof(taraddr))) {
        GLogger.LogErr(LOG_ERR, "connect() to proxy server error");
        return -1;
    }
    return tarfd;
}

bool Proxy::LoginProxyServer()
{
    char buf[MAXBUF];

    buf[0] = GConfig.Username.size();
    strncpy(buf + 1, GConfig.Username.c_str(), buf[0]);

    int pos = 1 + buf[0];
    buf[pos] = GConfig.Password.size();
    strncpy(buf + pos + 1, GConfig.Password.c_str(), buf[pos]);

    int len = 2 + buf[0] + buf[pos];
    if (len != encrypter->Write(buf, len)) {
        GLogger.LogErr(LOG_ERR, "write username/password error");
        return false;
    }

    if (1 != encrypter->Read(buf, 1) || buf[0] != 0) {
        GLogger.LogErr(LOG_ERR, "login proxy server failed");
        return false;
    }
    return true;
}

bool Proxy::ValidateProxyClient()
{
    char buf[MAXBUF];
    int readn = encrypter->Read(buf, sizeof(buf));
    if (readn < 1 || buf[0] <= 0) {
        GLogger.LogErr(LOG_NOTICE, "read username length error");
        return false;
    }

    int userlen = buf[0];
    if (readn < 1 + userlen) {
        GLogger.LogErr(LOG_NOTICE, "read usrename error");
        return false;
    }
    string username = string(buf + 1, userlen);

    if (readn < 1 + userlen + 1 || buf[1 + userlen] <= 0) {
        GLogger.LogErr(LOG_NOTICE, "read password length error");
        return false;
    }
    int pwdlen = buf[1 + userlen];
    if (readn < 1 + userlen + 1 + pwdlen) {
        GLogger.LogErr(LOG_NOTICE, "read password error");
        return false;
    }
    string passwd = string(buf + 1 + userlen + 1, pwdlen);

    char res[2];
    if (GPasswd.IsValidUser(username, passwd)) {
        res[0] = 0;
        if (1 != encrypter->Write(res, 1)) {
            GLogger.LogErr(LOG_ERR, "write auth result error");
            return false;
        }
        return true;
    }

    res[0] = 1;
    encrypter->Write(res, 1);
    return false;
}

int Proxy::ConnectRealServer(uint32_t ip, uint16_t port)
{
    struct sockaddr_in remoteaddr;
    bzero(&remoteaddr, sizeof(remoteaddr));
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_addr.s_addr = ip;
    remoteaddr.sin_port = htons(port);

    int remotefd = socket(AF_INET, SOCK_STREAM, 0);
    if (remotefd == -1) {
        GLogger.LogErr(LOG_ERR, "create socket to real server error");
        return -1;
    }

    if (connect(remotefd,
            (struct sockaddr *)&remoteaddr,
            sizeof(remoteaddr)) < 0) {
        GLogger.LogErr(LOG_ERR, "connect() to real server error");
        return -1;
    }

    return remotefd;
}

void Proxy::ForwardData(int srcfd, int tarfd)
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

int Proxy::ForwardData(int srcfd, int tarfd, bool fromClient)
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
