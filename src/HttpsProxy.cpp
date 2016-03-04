#include "HttpsProxy.h"
#include "Logger.h"
#include "Utils.h"
#include <vector>

using namespace std;

bool HttpsServerProxy::ParseIpPort(string &domain, uint32_t &ip, uint16_t &port)
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

    return true;
}

void HttpsServerProxy::Run(int srcfd, const char *request, int len)
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

    resstr += " 200 Connection Established\r\n\r\n";
    int reslen = resstr.size();
    if (reslen != encrypter->Write(resstr.c_str(), reslen)) {
        GLogger.LogErr(LOG_ERR, "write 200 response error");
        return;
    }

    ForwardData(srcfd, remotefd);
}

bool HttpsServerProxy::isMatch(const char *request, int len)
{
    return len >= 8 && strncasecmp(request, "CONNECT ", 8) == 0;
}
