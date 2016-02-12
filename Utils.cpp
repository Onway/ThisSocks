#include "Utils.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <syslog.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

bool Utils::TrimStart(string &str)
{
	int s = 0;
	for (; s < str.size(); ++s) {
		if (str[s] != ' ' && str[s] != '\t') {
			break;
		}
	}

	if (s == 0) {
		return false;
	}

	if (s >= str.size()) {
		str = "";
	} else {
		str = str.substr(s);
	}
	return true;
}

bool Utils::TrimEnd(string &str)
{
	int e = str.size() - 1;
	for (; e >= 0; --e) {
		if (str[e] != ' ' && str[e] != '\t') {
			break;
		}
	}

	if (e == str.size() - 1) {
		return false;
	}

	if (e < 0) {
		str = "";
	} else {
		str = str.substr(0, e + 1);
	}
	return true;
}

bool Utils::Trim(string &str)
{
	bool s = TrimStart(str);
	bool e = TrimEnd(str);
	return s || e;
}

void Utils::Split(const string &str, char c, vector<string> &vec)
{
	vec.clear();

	string::size_type s = 0, e = 0;
	for (; e < str.size(); ++e) {
		if (str[e] == c) {
			vec.push_back(str.substr(s, e - s));
			s = e + 1;
		}
	}

	if (e != 0 && s <= e) {
		vec.push_back(str.substr(s, e - s));
	}
}

string Utils::GetSocketPair(int connfd)
{
    struct sockaddr_in localAddr;
    struct sockaddr_in remoteAddr;
    socklen_t len;

    memset(&localAddr, 0, sizeof(localAddr));
    memset(&remoteAddr, 0, sizeof(remoteAddr));
    if (getsockname(connfd, (struct sockaddr *)&localAddr, &len) == -1) {
        GLogger.LogErr(LOG_ERR, "getsockname error");
    }
    if (getpeername(connfd, (struct sockaddr *)&remoteAddr, &len) == -1) {
        GLogger.LogErr(LOG_ERR, "getpeername error");
    }

    char localIp[20];
    char remoteIp[20];
    inet_ntop(AF_INET, &localAddr.sin_addr, localIp, 20);
    inet_ntop(AF_INET, &remoteAddr.sin_addr, remoteIp, 20);

    char buf[100];
    snprintf(buf, 100, "%s:%u-%s:%u",
        localIp, ntohs(localAddr.sin_port),
        remoteIp, ntohs(remoteAddr.sin_port));
    return string(buf);
}
