#include "HttpProxy.h"

using namespace std;

bool HttpProxy::isMatch(const char *request, int len)
{
    return true;
}

void HttpClientProxy::Run(int srcfd, const char *request, int len)
{
}

void HttpServerProxy::Run(int srcfd)
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
    string request = string(buf, len);
    uint32_t ip;
    uint16_t port;
    if (!ParseIpPort(request, ip, port)) {
        return;
    }
    GLogger.LogMsg(LOG_DEBUG, "ip-port: %d, %d", ip, port);

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

    string tmp;
    tmp.resize(request.size());
    transform(request.begin(), request.end(), tmp.begin(), ::tolower);
    string::size_type idx = tmp.find("proxy-connection");
    if (idx != string::npos) {
        request.erase(idx, 6);
    }
    len = request.size();
    if (len != write(remotefd, request.c_str(), len)) {
        GLogger.LogErr(LOG_ERR, "write request to server error");
        return;
    }

    ForwardData(srcfd, remotefd);
}

bool HttpServerProxy::ParseIpPort(string &request, uint32_t &ip, uint16_t &port)
{
    string::size_type sIdx = request.find("http://");
    if (sIdx != string::npos) {
        sIdx += 7;
    } else if ((sIdx = request.find(' ')) != string::npos) {
        sIdx += 1;
    } else {
        GLogger.LogMsg(LOG_DEBUG, "find http:// error");
        return false;
    }

    string::size_type eIdx = request.find("/", sIdx);
    if (eIdx == string::npos) {
        GLogger.LogMsg(LOG_DEBUG, "locate domain error");
        return false;
    }

    vector<string> parts;
    string domain = request.substr(sIdx, eIdx - sIdx);
    Utils::Split(domain, ':', parts);
    if (parts.size() != 1 && parts.size() != 2) {
        GLogger.LogMsg(LOG_DEBUG, "parse domain error");
        return false;
    } else if (parts.size() == 1) {
        port = 80;
    } else {
        port = atoi(parts[1].c_str());
    }

    struct hostent *hptr = gethostbyname(parts[0].c_str());
    if (hptr == NULL) {
        GLogger.LogErr(LOG_ERR, "gethostname error");
        return false;
    }
    ip = *((uint32_t *)*hptr->h_addr_list);
    return true;
}

void HttpServerProxy::Run(int srcfd, const char *request, int len)
{
}
