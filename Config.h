#ifndef CONFIGENTRY_H
#define CONFIGENTRY_H

#include <string>
#include "Utils.h"

class Config
{
public:
	int ServerPort;		
	int LocalPort;
	bool RunAsClient;
	bool RunAsDaemon;
	std::string ServerAddress;
	std::string LocalAddress;
	std::string CfgFile;
	std::string PwdFile;
	std::string Username;
	std::string Password;

	Config();
	int Init(int argc, char *argv[]);

private:
	bool LoadFromFile();
	bool CheckOptions();
	void PrintUsage();
	int ParseArguments(int argc, char *argv[]);
	bool ParseOption(const std::string &key, const std::string &value);
};

extern Config GConfig;

#endif
