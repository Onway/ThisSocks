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
	std::string GetPassword(std::string user);

protected:
	std::map<std::string, std::string> mp;
};

extern Passwd GPasswd;

#endif
