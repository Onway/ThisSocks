#include "Config.h"
#include "Utils.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <utility>

using namespace std;

Config GConfig;

Config::Config()
    : ServerPort(0), LocalPort(0), StatPort(0),
	RunAsClient(false), RunAsDaemon(false),
	_D(false)
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

	if (_D) { // debug mode enabled
		RunAsDaemon = false;
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
		} else if (strcmp(argv[idx], "-D") == 0) {
			_D = true;	
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
	} else if (key == "StatAddress") {
		StatAddress = value;
	} else if (key == "StatPort") {
		StatPort = atoi(value.c_str());
	} else if (key == "PwdFile") {
		string dir = Utils::GetAbsDir(CfgFile);
		PwdFile = Utils::JoinPath(dir, value);
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

bool Config::CheckOptions() const
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

void Config::PrintUsage() const
{
	cout << "ThisSocks v2.3" << endl;
	cout << endl;
	cout << "Usage: ThisSocks [-D] <-C FILE>" << endl;
	cout << "       ThisSocks -h" << endl;
	cout << endl;
	cout << "    -C FILE        path to config file" << endl;
	cout << "    -D             debug mode" << endl;
	cout << "    -h             show this usage and exit" << endl;
	cout << endl;
}

