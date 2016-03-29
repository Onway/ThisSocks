#include "ClientProxy.h"
#include "Logger.h"
#include "Config.h"

void ClientProxy::Process(int srcfd)
{
	int len;
	char request[MAXBUF];

	if ((len = read(srcfd, request, sizeof(request))) < 0) {
		GLogger.LogErr(LOG_NOTICE, "read proxy request error");
		return;
	} else if (len == 0) {
		// occurs when using system proxy setting in ubuntu 14.04
		GLogger.LogMsg(LOG_DEBUG, "proxy request connection closed");
		return;
	}

	int tarfd = ConnectProxyServer();
	if (tarfd < 0) {
		return;
	}

	do {
		encrypter = GEncryptFactory.GetEncrypter();
		if (!encrypter->SetClientFd(tarfd)) {
			break;
		}

		if (!LoginProxyServer()) {
			break;
		}

		if (len != encrypter->Write(request, len)) {
			GLogger.LogErr(LOG_ERR, "write request to server error");
			break;
		}

		ForwardData(srcfd, tarfd);
	} while (0);

	close(tarfd);
}

int ClientProxy::ConnectProxyServer() const
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

bool ClientProxy::LoginProxyServer() const
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
