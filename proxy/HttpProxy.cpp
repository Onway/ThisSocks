#include "HttpProxy.h"

using namespace std;

// defaul proxy
bool HttpProxy::isMatch(const char *, int)
{
    return true;
}

void HttpClientProxy::Run(int srcfd, const char *request, int len)
{
    int tarfd = encrypter->GetFd();
    if (len != encrypter->Write(request, len)) {
        GLogger.LogErr(LOG_ERR, "write request to server error");
        return;
    }
    ForwardData(srcfd, tarfd);
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
    string reqstr = string(request, len);
    uint32_t ip;
    uint16_t port;
    if (!ParseIpPort(reqstr, ip, port)) {
        return;
    }
    GLogger.LogMsg(LOG_DEBUG, "ip-port: %d, %d", ip, port);

    int remotefd = ConnectRealServer(ip, port);
    if (remotefd < 0) {
        return;
    }

    string tmp;
    tmp.resize(reqstr.size());
    transform(reqstr.begin(), reqstr.end(), tmp.begin(), ::tolower);
    string::size_type idx = tmp.find("proxy-connection");
    if (idx != string::npos) {
        reqstr.erase(idx, 6);
    }
    len = reqstr.size();
    if (len != write(remotefd, reqstr.c_str(), len)) {
        GLogger.LogErr(LOG_ERR, "write request to real server error");
        return;
    }

    ForwardData(srcfd, remotefd);
}
