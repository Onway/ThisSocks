#ifndef LOGGER_H
#define LOGGER_H

#include <syslog.h>
#include <cstdio>
#include <string>

#define LOGMAX 1024

class Logger
{
public:
	Logger();
	void Init(const char *ident);
	void LogMsg(int priority, const char *format, ...) const;
	void LogErr(int priority, const char *format, ...) const;

private:
	void Log(bool logerr, int priority, const char *fmt, va_list ap) const;
	void AppendPreix(std::string &logline) const;
	void AppendLevel(std::string &logline, int priority) const;
	const char *ident;
};

extern Logger GLogger;

#endif
