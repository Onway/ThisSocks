#include "http_proxy.h"
#include "logger.h"
#include "utils.h"
#include "recorder.h"

using namespace std;

void HttpServerProxy::Process(int srcfd, const char *request, int len) const
{
    string reqstr = string(request, len);
    uint32_t ip;
    uint16_t port;
    if (!ParseIpPort(reqstr, ip, port)) {
        return;
    }

    int remotefd = ConnectRealServer(ip, port);
    if (remotefd < 0) {
        return;
    }

	do {
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
			break;
		}

		Recorder::RecordUpload(len);
		ForwardData(srcfd, remotefd);
	} while (0);

	close(remotefd);
}

bool HttpServerProxy::ParseIpPort(string &request, uint32_t &ip, uint16_t &port) const
{
    string::size_type sIdx = request.find("http://");
    if (sIdx != string::npos) {
        sIdx += 7;
    } else if ((sIdx = request.find(' ')) != string::npos) {
        sIdx += 1;
    } else {
        GLogger.LogMsg(LOG_NOTICE, "find http:// error");
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
	ip = GetIPv4ByName(parts[0]);

    return true;
}

bool HttpServerProxy::isMatch(const char *, int)
{
    return true;
}
