#include "SocksProxy.h"
#include "Logger.h"

using namespace std;

void SocksServerProxy::Process(int srcfd, const char*, int) const
{
    char buf[MAXBUF] = { 5, 0 };
    if (2 != encrypter->Write(buf, 2)) {
        GLogger.LogErr(LOG_ERR, "write socks proxy response error");
        return;
    }

    uint32_t ip;
    uint16_t port;
    int readn = encrypter->Read(buf, sizeof(buf));
    if (readn < 4) {
        GLogger.LogErr(LOG_NOTICE, "read socks connect request length error");
        return;
    }

    char expect[4] = { 5, 1, 0 };
    if (0 != memcmp(buf, expect, 3)) {
        GLogger.LogMsg(LOG_DEBUG, "socks connect request invalid");
        return;
    }
    if (buf[3] == 1) {
        if (readn < 10) {
            GLogger.LogMsg(LOG_NOTICE, "read ip,port length error");
            return;
        }
        ip = *((uint32_t *)&buf[4]);
        port = *((uint16_t *)&buf[8]);
    } else if (buf[3] == 3) {
        if (readn < 5) {
            GLogger.LogMsg(LOG_NOTICE, "read host length error");
            return;
        }
        int hostlen = buf[4];
        if (readn < 4 + 1 + hostlen + 2) {
            GLogger.LogErr(LOG_NOTICE, "read host,port length error");
            return;
        }
        port = *((uint16_t *)&buf[readn - 2]);

        buf[5 + hostlen] = 0;
        struct hostent *hptr = gethostbyname(buf + 5);
        if (hptr == NULL) {
            GLogger.LogErr(LOG_ERR, "gethostname error");
            return;
        }
        ip = *((uint32_t *)*hptr->h_addr_list);
    } else {
		GLogger.LogMsg(LOG_NOTICE, "unsupported socks address type");
		return;
	}
    port = ntohs(port);

    int remotefd = ConnectRealServer(ip, port);
    if (remotefd < 0) {
        return;
    }

    char res[10];
    memset(res, 0, 10);
    res[0] = 5;
    res[3] = 1;
    if (10 != encrypter->Write(res, 10)) {
        GLogger.LogErr(LOG_ERR, "write connect response error");
        return;
    }

    ForwardData(srcfd, remotefd);
}

bool SocksServerProxy::isMatch(const char *request, int len)
{
	if (len < 2 || request[0] != 5) {
		return false;
	}

	bool found = false;
	int cnt = request[1];
	for (int i = 0; i < cnt && i + 2 < len; ++i) {
		if (request[i + 2] == 0) {
			found = true;
			break;
		}
	}
	return found;
}
