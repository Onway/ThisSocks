#include "https_proxy.h"
#include "logger.h"
#include "utils.h"
#include <vector>

using namespace std;

void HttpsServerProxy::Process(int srcfd, const char *request, int len) const
{
    int i = 0;
    for (i = 8; i < len; ++i) {
        if (request[i] == ' ') {
            break;
        }
    }

    uint32_t ip;
    uint16_t port;
    string domain = string(request + 8, i - 8);
    if (!ParseIpPort(domain, ip, port)) {
        return;
    }

    char proto[100];
    int j;
    for (j = 0, i = i + 1; j < 99 && i < len - 1; ++i, ++j) {
        if (request[i] == '\r' && request[i + 1] == '\n') {
            break;
        }
        proto[j] = request[i];
    }
    string resstr = string(proto, j);

    int remotefd = ConnectRealServer(ip, port);
    if (remotefd < 0) {
        return;
    }

	do {
		resstr += " 200 Connection Established\r\n\r\n";
		int reslen = resstr.size();
		if (reslen != encrypter->Write(resstr.c_str(), reslen)) {
			GLogger.LogErr(LOG_ERR, "write 200 response error");
			break;
		}

		ForwardData(srcfd, remotefd);
	} while (0);

	close(remotefd);
}

bool HttpsServerProxy::ParseIpPort(string &domain, uint32_t &ip, uint16_t &port) const
{
    vector<string> part;
    Utils::Split(domain, ':', part);
    if (part.size() != 2) {
        GLogger.LogMsg(LOG_DEBUG, "read hostname port error");
        return false;
    }

	ip = GetIPv4ByName(part[0]);
    port = atoi(part[1].c_str());

    return true;
}

bool HttpsServerProxy::isMatch(const char *request, int len)
{
    return len >= 8 && strncasecmp(request, "CONNECT ", 8) == 0;
}
