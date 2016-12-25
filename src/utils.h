#ifndef THISSOCKS_UTILS_H_
#define THISSOCKS_UTILS_H_

#include <iostream>
#include <vector>

class Utils
{
public:
    static void TrimLeft(std::string& s);
    static void TrimRight(std::string& s);
	static void Trim(std::string& s);

	static void Split(const std::string& str,
			char c, std::vector<std::string>& vec);

    static std::string GetAbsolutePath(const std::string& path);
    static std::string GetDirectoryName(const std::string& path);
	static std::string GetAbsDir(const std::string& filePath);
	static std::string JoinPath(const std::string& dir, const std::string& path);

    static std::string GetSocketPair(int connfd);
};
#endif  // THISSOCKS_UTILS_H_
