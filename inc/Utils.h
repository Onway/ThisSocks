#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>

class Utils
{
public:
	static bool TrimStart(std::string &str);
	static bool TrimEnd(std::string &str);
	static bool Trim(std::string &str);
	static void Split(
			const std::string &str, char c, std::vector<std::string> &vec);

    static std::string GetSocketPair(int connfd);
	static std::string GetAbsDir(std::string filePath);
};
#endif
