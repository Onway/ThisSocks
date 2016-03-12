#include "Passwd.h"
#include "Config.h"
#include "Logger.h"
#include "Utils.h"

using namespace std;

Passwd GPasswd;

void Passwd::LoadFile(string fPath)
{
	mp.clear();
	ifstream ifs(fPath.c_str());
	if (!ifs) {
		GLogger.LogMsg(LOG_ERR, "open passwd file error");
		return;
	}

	string line;
	vector<string> vec;
	while (getline(ifs, line)) {
		Utils::Trim(line);

		if (line.empty() || line[0] == '#') {
			continue;
		}

		Utils::Split(line, ':', vec);
		if (vec.size() != 2) {
			GLogger.LogMsg(LOG_WARNING,
					"unknown passwd line: %s", line.c_str());
			continue;
		}

		Utils::Trim(vec[0]);
		Utils::Trim(vec[1]);
		if (vec[0].empty() || vec[1].empty()) {
			GLogger.LogMsg(LOG_WARNING, "invalid passwd line: %s", line.c_str());
			continue;
		}
		mp[vec[0]] = vec[1];
	}
	ifs.close();
}

bool Passwd::IsValidUser(string user, string pwd)
{
	map<string, string>::iterator iter = mp.find(user);
	return iter != mp.end() && iter->second == pwd;
}
