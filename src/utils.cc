#include "utils.h"

#include <errno.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <arpa/inet.h>
#include <limits.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "feature.h"

using namespace std;

bool Utils::TrimStart(string &str)
{
	int s = 0;
    int len = str.size();
    for (; s < len; ++s) {
		if (str[s] != ' ' && str[s] != '\t') {
			break;
		}
	}

	if (s == 0) {
		return false;
	}

    if (s >= len) {
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

    if (e == (int)str.size() - 1) {
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
	char buf[1024];
    struct sockaddr_in localAddr;
    struct sockaddr_in remoteAddr;
    socklen_t len;

    memset(&localAddr, 0, sizeof(localAddr));
    memset(&remoteAddr, 0, sizeof(remoteAddr));
    if (getsockname(connfd, (struct sockaddr *)&localAddr, &len) == -1) {
		strerror_r(errno, buf, sizeof(buf));
		return string("getsockname error: ") + buf;
    }
    if (getpeername(connfd, (struct sockaddr *)&remoteAddr, &len) == -1) {
		strerror_r(errno, buf, sizeof(buf));
		return string("getpeername error: ") + buf;
    }

    char localIp[20];
    char remoteIp[20];
    inet_ntop(AF_INET, &localAddr.sin_addr, localIp, sizeof(localIp));
    inet_ntop(AF_INET, &remoteAddr.sin_addr, remoteIp, sizeof(remoteIp));

    snprintf(buf, sizeof(buf), "%s:%u-%s:%u",
        localIp, ntohs(localAddr.sin_port),
        remoteIp, ntohs(remoteAddr.sin_port));
    return string(buf);
}

/*
 * 返回指定路径所在目录的绝对路径
 * 如果传入的是非根目录，截掉最后的'/'返回
 * 参数无效返回空串
 */
string Utils::GetAbsDir(string path)
{
	char rpath[4096];
	if (realpath(path.c_str(), rpath) == NULL) {
		return "";
	}
	string strpath = string(rpath);	

	struct stat sbuf;
	if (stat(strpath.c_str(), &sbuf) == -1) {
		return "";
	}

	if (S_ISDIR(sbuf.st_mode)) {
		return strpath.size() == 1 ? strpath : strpath + "/";
	}
	string::size_type lastpos = strpath.find_last_of('/');
	return strpath.substr(0, lastpos + 1);
}

/*
 * 将指定的dir和path组合成绝对路径
 * dir必须是绝对路径
 * 如果path是绝对路径则返回path
 * 参数无效返回空串
 */
string Utils::JoinPath(string dir, string path)
{
	if (path.empty()) {
		return "";
	}
	if (path[0] == '/' && path.size() == 1) {
		return path;
	} 
	if (path[0] == '/') {
		return path[path.size() - 1] == '/' ?
			path.substr(0, path.size() - 1) : path;
	}

	string ret;
	if (dir.empty() || dir[0] != '/') {
		return "";
	}
	if (dir.size() == 1) {
		ret = dir + path;
	} else if (dir[dir.size() - 1] == '/') {
		ret = dir + path;
	} else {
		ret = dir + "/" + path;
	}

	return ret[ret.size() - 1] == '/' ?
		ret.substr(0, ret.size() - 1) : ret;
}
