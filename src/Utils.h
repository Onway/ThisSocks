#ifndef THISSOCKS_UTILS_H_
#define THISSOCKS_UTILS_H_

#include <iostream>
#include <vector>

class Utils
{
public:
	static bool TrimStart(std::string &str);
	static bool TrimEnd(std::string &str);
	static bool Trim(std::string &str);
	static void Split(const std::string &str,
			char c, std::vector<std::string> &vec);

	static std::string GetAbsDir(std::string filePath);
	static std::string JoinPath(std::string dir, std::string path);

    static std::string GetSocketPair(int connfd);
};
#endif  // THISSOCKS_UTILS_H_
