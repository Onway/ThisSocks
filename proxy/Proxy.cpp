#include "Proxy.h"

using namespace std;

void Proxy::Run(int srcfd)
{
    char request[MAXBUF];
    int len;
    Proxy* proxy = NULL;
    EncryptBase* encry = NULL;
    do {
        if (GConfig.RunAsClient) {
            if ((len = read(srcfd, request, sizeof(request))) < 0) {
                GLogger.LogErr(LOG_NOTICE, "read proxy request error");
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

            encry = GEncryptFactory.GetEncrypter();
            if (!encry->SetClientFd(serverfd)) {
                break;
            }
        } else {
            encry = GEncryptFactory.GetEncrypter();
            if (!encry->SetServerFd(srcfd)) {
                break;
            }

            if ((len = encry->Read(request, sizeof(request))) < 0) {
                GLogger.LogErr(LOG_NOTICE, "read proxy request error");
                break;
            }

            proxy = SelectLocalProxy(!GConfig.RunAsClient, request, len);
            if (proxy == NULL) {
                break;
            }
        }

        proxy->encrypter = encry;
        proxy->Run(srcfd, request, len);
    } while (false);

    if (encry != NULL) {
        delete encry;
    }
    if (proxy != NULL) {
        delete proxy;
    }
}

Proxy* Proxy::SelectLocalProxy(bool isClient, const char *request, int len)
{
    return 0;
}

int Proxy::ConnectProxyServer()
{
    return 0;
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
            }
		}

        if (FD_ISSET(tarfd, &fdset)) {
            if ((ret = ForwardData(tarfd, srcfd, false)) < 0) {
                break;
            } else if (ret == 0 && halfClose) {
                break;
            } else if (ret == 0) {
                halfClose = true;
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

int Proxy::ReadN(int fd, void *buf, size_t count)
{
    int readn;
    size_t sum = 0;

    while (sum < count) {
        readn = read(fd, (char*)buf + sum, count - sum);
        if (readn < 0) {
            if (errno == EINTR) {
                continue;
            }
            return readn;
        } else if (readn == 0) { // EOF
            break;
        }
        sum += readn;
    }

    if (sum < count) {
        GLogger.LogMsg(LOG_NOTICE,
            "ReadN return %d for request %d bytes", sum, count);
    }
    return sum;
}
