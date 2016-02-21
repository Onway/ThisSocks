#include "HttpsProxy.h"

using namespace std;

bool HttpsProxy::isMatch(const char *request, int len)
{
    return true;
}

void HttpsClientProxy::Run(int srcfd)
{
    /*
    int tarfd = ConnectServer();
    if (tarfd < 0) {
        return;
    }

    encrypter = GEncryptFactory.GetEncrypter();
    if (!encrypter->SetClientFd(tarfd)) {
        return;
    }

    ForwardData(srcfd, tarfd);
    */
}

void HttpsClientProxy::Run(int srcfd, const char *request, int len)
{
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

    GLogger.LogMsg(LOG_DEBUG, "domain: %d:%d", ip, port);
    return true;
}

void HttpsServerProxy::Run(int srcfd, const char *request, int len)
{
}
