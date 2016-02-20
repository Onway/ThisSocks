#include "Proxy.h"

#define MAXBUF 8192

using namespace std;

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


SocksServerProxy::SocksServerProxy()
{
	pwd.LoadFile(GConfig.PwdFile);
}

void SocksServerProxy::Run(int srcfd)
{
    encrypter = GEncryptFactory.GetEncrypter();
    if (!encrypter->SetServerFd(srcfd)) {
        return;
    }

    if (!SelectMethod()) {
        return;
    }

    if (!ValidateSource()) {
        return;
    }

    int tarfd;
    if ((tarfd = ConnectServer()) < 0) {
        return;
    }

    ForwardData(srcfd, tarfd);
}

bool SocksServerProxy::SelectMethod()
{
    char buf[3];
    int readn;
    if (3 != (readn = encrypter->Read(buf, sizeof(buf)))) {
        GLogger.LogErr(LOG_NOTICE, "read proxy request length error");
        return false;
    }

    if (buf[0] != 5 || buf[1] != 1 || buf[2] != 2) {
        string sockPair = Utils::GetSocketPair(encrypter->GetFd());
        GLogger.LogMsg(LOG_NOTICE, "proxy request invalid(%s)", sockPair.c_str());
        return false;
    }

    buf[1] = 2;
    if (2 != encrypter->Write(buf, 2)) {
        GLogger.LogErr(LOG_ERR, "write proxy response error");
        return false;
    }
    return true;
}

int SocksServerProxy::ConnectServer()
{
    char buf[MAXBUF];
	uint32_t ip; 
    uint16_t port;
    int readn = encrypter->Read(buf, sizeof(buf));
	if (readn < 4) {
        GLogger.LogErr(LOG_NOTICE, "read connect request length error");
		return -1;
	}

	char expect[4] = { 5, 1, 0 };
	if (0 != memcmp(buf, expect, 3)) {
        GLogger.LogMsg(LOG_DEBUG, "connect request invalid");
        return -1;
	}
    if (buf[3] == 1) {
        if (readn < 10) {
            GLogger.LogErr(LOG_NOTICE, "read ip,port length(%d) error");
            return -1;
        }
        ip = *((uint32_t *)&buf[4]);
        port = *((uint16_t *)&buf[8]);
    } else if (buf[3] == 3) {
        if (readn < 5) {
            GLogger.LogErr(LOG_NOTICE, "read host length error");
            return -1;
        }
        int hostlen = buf[4];
        if (readn < 4 + 1 + hostlen + 2) {
            GLogger.LogErr(LOG_NOTICE, "read host,port length error");
            return -1;
        }
        port = *((uint16_t *)&buf[readn - 2]);

        buf[5 + hostlen] = 0;
        struct hostent *hptr = gethostbyname(buf + 5);
		if (hptr == NULL) {
            GLogger.LogErr(LOG_ERR, "gethostname error");
			return -1;
		}
		ip = *((uint32_t *)*hptr->h_addr_list);
	}

	struct sockaddr_in remoteaddr;
	bzero(&remoteaddr, sizeof(remoteaddr));
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = ip;
	remoteaddr.sin_port = port;

	int remotefd = socket(AF_INET, SOCK_STREAM, 0);
	if (remotefd == -1) {
        GLogger.LogErr(LOG_ERR, "create socket to remote error");
		return -1;
	}

	if (connect(remotefd,
			(struct sockaddr *)&remoteaddr,
		   	sizeof(remoteaddr)) < 0) {
        GLogger.LogErr(LOG_ERR, "connect to remote error");
		return -1;
	}

	char res[10];
	memset(res, 0, 10);
	res[0] = 5;
	res[3] = 1;
    if (10 != encrypter->Write(res, 10)) {
        GLogger.LogErr(LOG_ERR, "write connect response error");
		return -1;
	}

	return remotefd;
}

bool SocksServerProxy::ValidateSource()
{
    char buf[MAXBUF];
    int readn = encrypter->Read(buf, sizeof(buf));
    if (readn < 2 || buf[1] <= 0) {
        GLogger.LogErr(LOG_NOTICE, "read username length error");
		return false;
	}

    int userlen = buf[1];
    if (readn < 2 + userlen) {
        GLogger.LogErr(LOG_NOTICE, "read usrename error");
		return false;
	}
    string username = string(buf + 2, userlen);

    if (readn < 2 + userlen + 1 || buf[2 + userlen] <= 0) {
        GLogger.LogErr(LOG_NOTICE, "read password length error");
        return false;
    }
    int pwdlen = buf[2 + userlen];
    if (readn < 2 + userlen + 1 + pwdlen) {
        GLogger.LogErr(LOG_NOTICE, "read password error");
        return false;
    }
    string passwd = string(buf + 2 + userlen + 1, pwdlen);

	char res[2] = { 1, 0 };
	if (pwd.IsValidUser(username, passwd)) {
        if (2 != encrypter->Write(res, 2)) {
            GLogger.LogErr(LOG_ERR, "write auth result error");
			return false;
		}
		return true;
	}
	return false;
}


void SocksClientProxy::Run(int srcfd)
{
    if (!WaitingMethod(srcfd)) {
        return;
    }

    int tarfd = ConnectServer();
    if (tarfd < 0) {
        return;
    }

    encrypter = GEncryptFactory.GetEncrypter();
    if (!encrypter->SetClientFd(tarfd)) {
        return;
    }

    if (!RequestProxy()) {
        return;
    }

    if (!ResponseMethod(srcfd)) {
        return;
    }

    ForwardData(srcfd, tarfd);
}

bool SocksClientProxy::WaitingMethod(int srcfd)
{
    char buf[3];
    if (3 != ReadN(srcfd, buf, sizeof(buf))) {
        GLogger.LogErr(LOG_NOTICE, "read proxy request length error");
        return false;
    }

    if (buf[0] != 5 || buf[1] != 1 || buf[2] != 0) {
        GLogger.LogMsg(LOG_NOTICE, "proxy request invalid");
        return false;
    }

    return true;
}

bool SocksClientProxy::ResponseMethod(int srcfd)
{
    char buf[2] = { 5, 0 };
    if (2 != write(srcfd, buf, sizeof(buf))) {
        GLogger.LogErr(LOG_ERR, "write proxy response error");
        return false;
    }
    return true;
}

int SocksClientProxy::ConnectServer()
{
    int tarfd;
    struct sockaddr_in taraddr;

    if ((tarfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        GLogger.LogErr(LOG_ERR, "create socket to server error");
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
        GLogger.LogErr(LOG_ERR, "connect to server error");
		return -1;
	}
    return tarfd;
}

bool SocksClientProxy::RequestProxy()
{
    char buf[256 + 256 + 3] = { 5, 1, 2 };
    if (3 != encrypter->Write(buf, 3)) {
        GLogger.LogErr(LOG_ERR, "write proxy request to server error");
        return false;
    }

    if (2 != encrypter->Read(buf, sizeof(buf))) {
        GLogger.LogErr(LOG_ERR, "read proxy response from server error");
        return false;
	}
	if (buf[0] != 5 || buf[1] != 2) {
        GLogger.LogErr(LOG_ERR, "server response proxy content invalid");
		return -1;
	}
	
	buf[0] = 1;
	buf[1] = GConfig.Username.size();
	strncpy(buf + 2, GConfig.Username.c_str(), buf[1]);
	
    int pos = 2 + buf[1];
	buf[pos] = GConfig.Password.size();
	strncpy(buf + pos + 1, GConfig.Password.c_str(), buf[pos]);

	int len = 3 + buf[1] + buf[pos];
    if (len != encrypter->Write(buf, len)) {
        GLogger.LogErr(LOG_ERR, "write username/password error");
        return false;
	}

    if (2 != encrypter->Read(buf, 2) || buf[0] != 1 || buf[1] != 0) {
        GLogger.LogErr(LOG_ERR, "username/password auth result error");
        return false;
	}
    return true;
}


void HttpsClientProxy::Run(int srcfd)
{
    int tarfd = ConnectServer();
    if (tarfd < 0) {
        return;
    }

    encrypter = GEncryptFactory.GetEncrypter();
    if (!encrypter->SetClientFd(tarfd)) {
        return;
    }

    ForwardData(srcfd, tarfd);
}


void HttpsServerProxy::Run(int srcfd)
{
    encrypter = GEncryptFactory.GetEncrypter();
    if (!encrypter->SetServerFd(srcfd)) {
        return;
    }

    char buf[MAXBUF];
    int len;
    if ((len = encrypter->Read(buf, sizeof(buf))) < 0) {
        GLogger.LogMsg(LOG_DEBUG, "read https request error");
        return;
    }

    if (len < 8 || strncmp(buf, "CONNECT ", 8) != 0) {
        GLogger.LogMsg(LOG_DEBUG, "read CONNECT error");
        return;
    }

    int i = 0;
    for (i = 8; i < len; ++i) {
        if (buf[i] == ' ') {
            break;
        }
    }

    uint32_t ip;
    uint16_t port;
    string domain = string(buf + 8, i - 8);
    if (!ParseIpPort(domain, ip, port)) {
        return;
    }

    char proto[100];
    int j;
    for (j = 0, i = i + 1; j < 99 && i < len - 1; ++i, ++j) {
        if (buf[i] == '\r' && buf[i + 1] == '\n') {
            break;
        }
        proto[j] = buf[i];
    }
    string resstr = string(proto, j);

    struct sockaddr_in remoteaddr;
    bzero(&remoteaddr, sizeof(remoteaddr));
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_addr.s_addr = ip;
    remoteaddr.sin_port = htons(port);

    int remotefd = socket(AF_INET, SOCK_STREAM, 0);
    if (remotefd == -1) {
        GLogger.LogErr(LOG_ERR, "create socket to remote error");
        return;
    }

    if (connect(remotefd,
            (struct sockaddr *)&remoteaddr,
            sizeof(remoteaddr)) < 0) {
        GLogger.LogErr(LOG_ERR, "connect to remote error");
        return;
    }

    resstr += " 200 Connection Established\r\n\r\n";
    int reslen = resstr.size();
    if (reslen != encrypter->Write(resstr.c_str(), reslen)) {
        GLogger.LogErr(LOG_ERR, "write 200 response error");
        return;
    }

    ForwardData(srcfd, remotefd);
}

bool HttpsServerProxy::ParseIpPort(string domain, uint32_t &ip, uint16_t &port)
{
    vector<string> part;
    Utils::Split(domain, ':', part);
    if (part.size() != 2) {
        GLogger.LogMsg(LOG_DEBUG, "read hostname port error");
        return false;
    }

    struct hostent *hptr = gethostbyname(part[0].c_str());
    if (hptr == NULL) {
        GLogger.LogErr(LOG_ERR, "gethostname error");
        return false;
    }
    ip = *((uint32_t *)*hptr->h_addr_list);
    port = atoi(part[1].c_str());

    GLogger.LogMsg(LOG_DEBUG, "domain: %d:%d", ip, port);
    return true;
}
