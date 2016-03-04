#ifndef PASSWD_H
#define PASSWD_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>

class Passwd
{
public:
    Passwd();
	void LoadFile(std::string fPath);
	bool IsValidUser(std::string user, std::string pwd);

protected:
	std::map<std::string, std::string> mp;
    bool hasLoadFile;
};

#endif
