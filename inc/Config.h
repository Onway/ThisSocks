#ifndef CONFIGENTRY_H
#define CONFIGENTRY_H

#include <string>

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
	bool _D; // debug mode

	int ParseArguments(int argc, char *argv[]);
	bool LoadFromFile();
	bool ParseOption(const std::string &key, const std::string &value);
	bool CheckOptions() const;
	void PrintUsage() const;
};

extern Config GConfig;

#endif
