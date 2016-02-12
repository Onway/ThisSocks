#ifndef PASSWD_H
#define PASSWD_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>

class Passwd
{
public:
	void LoadFile(std::string fPath);
	bool IsValidUser(std::string user, std::string pwd);

protected:
	std::map<std::string, std::string> mp;
};

#endif
