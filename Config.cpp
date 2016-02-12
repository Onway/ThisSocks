#include "Config.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <utility>

using namespace std;

Config GConfig;

Config::Config()
	: RunAsClient(false), RunAsDaemon(false), ServerPort(0), LocalPort(0)
{
}

int Config::Init(int argc, char *argv[])
{
	int ret = ParseArguments(argc, argv);
	if (ret > 0 && !CfgFile.empty()) {
		if (!LoadFromFile() || !CheckOptions()) {
			ret = -1;
		}
	}
	return ret;
}

int Config::ParseArguments(int argc, char *argv[])
{
	int idx = 1;
	while (idx < argc) {
		if (strcmp(argv[idx], "-C") == 0 && idx + 1 < argc) {
			CfgFile = argv[++idx];
		} else if (strcmp(argv[idx], "-h") == 0) {
			PrintUsage();
			return 0;
		} else {
			PrintUsage();
			return -1;
		}
		++idx;
	}

	if (CfgFile.empty()) {
		PrintUsage();
		return -1;
	}
	return 1;
}

bool Config::LoadFromFile()
{
	if (CfgFile.empty()) {
		cerr << "config file not specified" << endl;
		return false;
	}

	ifstream ifs(CfgFile.c_str());
	if (!ifs) {
		cerr << "open config file error" << endl;
		return false;
	}

	string line;
	vector<string> vec;
	while (getline(ifs, line)) {
		Utils::Trim(line);

		if (line.empty() || line[0] == '#') {
			continue;
		}

		Utils::Split(line, '=', vec);
		if (vec.size() != 2) {
			cerr << "unknown config line: " << line << endl;
			continue;
		}

		Utils::Trim(vec[0]);
		Utils::Trim(vec[1]);
		if (!ParseOption(vec[0], vec[1])) {
			cerr << "unknow config option: " << line << endl;
			continue;
		}
	}
	ifs.close();
	return true;
}

bool Config::ParseOption(const string &key, const string &value)
{
	if (key == "ServerAddress") {
		ServerAddress = value;		
	} else if (key == "ServerPort") {
		ServerPort = atoi(value.c_str());
	} else if (key == "LocalAddress") {
		LocalAddress = value;
	} else if (key == "LocalPort") {
		LocalPort = atoi(value.c_str());
	} else if (key == "PwdFile") {
		PwdFile = value;
	} else if (key == "Username") {
		Username = value;
	} else if (key == "Password") {
		Password = value;
	} else if (key == "RunAsClient") {
		RunAsClient = atoi(value.c_str()) != 0 ? true : false;
	} else if (key == "RunAsDaemon") {
		RunAsDaemon = atoi(value.c_str()) != 0 ? true : false;
	} else {
		return false;
	}
	return true;
}

void Config::PrintUsage()
{
	cout << "ThisSocks v0.1" << endl;
	cout << endl;
	cout << "Usage: ThisSocks <-C FILE | -h>" << endl;
	cout << "    -C FILE        path to config file" << endl;
	cout << "    -h             show this usage and exit" << endl;
	cout << endl;
}

bool Config::CheckOptions()
{
	bool isValid = true;
	if (RunAsClient) {
		if (LocalAddress.empty()) {
			cerr << "ClientAddress not specified" << endl;
			isValid = false;
		}
		if (LocalPort <= 0) {
			cerr << "ClientPort not specified" << endl;
			isValid = false;
		}
		if (Username.empty()) {
			cerr << "Username not specified" << endl;
			isValid = false;
		}
		if (Password.empty()) {
			cerr << "Password not specified" << endl;
			isValid = false;
		}
	} else {
		if (PwdFile.empty()) {
			cerr << "PwdFile not specified" << endl;
			isValid = false;
		}
	}

	if (ServerAddress.empty()) {
		cerr << "ServerAddress not specified" << endl;
		isValid = false;
	}
	if (ServerPort <= 0) {
		cerr << "ServerPort not specified" << endl;
		isValid = false;
	}

	return isValid;
}
