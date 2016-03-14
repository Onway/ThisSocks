#include "Feature.h"
#include "Logger.h"
#include <ctime>
#include <cstdarg>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

using namespace std;

Logger GLogger;

Logger::Logger() : ident(0)
{

}

void Logger::Init(const char *ident)
{
	if (ident != 0) {
		this->ident = ident;
		openlog(ident, LOG_PID, LOG_USER);
	}
}

void Logger::LogMsg(int priority, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Log(false, priority, fmt, ap);
	va_end(ap);	
}

void Logger::LogErr(int priority, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Log(true, priority, fmt, ap);
	va_end(ap);	
}

void Logger::Log(bool logerr, int priority, const char *fmt, va_list ap)
{
	char logbuf[LOGMAX];
	vsnprintf(logbuf, LOGMAX, fmt, ap);

	string logline;
	if (ident == 0) {
		AppendPreix(logline);
	}
	AppendLevel(logline, priority);
	logline += logbuf;

	char errbuf[LOGMAX];
	if (logerr) {
		strerror_r(errno, errbuf, sizeof(errbuf));
		if (ident == 0) {
			printf("%s: %s\n", logline.c_str(), errbuf);
		} else {
			syslog(priority,"%s: %s", logline.c_str(), errbuf);
		}
	} else {
		if (ident == 0) {
			printf("%s\n", logline.c_str());
		} else {
			syslog(priority, "%s", logline.c_str());
		}
	}
}

void Logger::AppendPreix(string &logline)
{
	time_t t = time(0);
	char buf[50];
	struct tm tm;
	localtime_r(&t, &tm);
	strftime(buf, sizeof(buf), "%b %d %T ", &tm);
	logline += buf;

	snprintf(buf, sizeof(buf), "[%d]: ", getpid());
	logline += buf;
}

void Logger::AppendLevel(string &logline, int priority)
{
	logline += "(";
	switch (priority) {
		case LOG_EMERG:
			logline += "EMERG";
			break;
		case LOG_ALERT:
			logline += "ALERT";
			break;
		case LOG_CRIT:
			logline += "CRIT";
			break;
		case LOG_ERR:
			logline += "ERR";
			break;
		case LOG_WARNING:
			logline += "WARNING";
			break;
		case LOG_NOTICE:
			logline += "NOTICE";
			break;
		case LOG_INFO:
			logline += "INFO";
			break;
		case LOG_DEBUG:
			logline += "DEBUG";
			break;
		default:
			break;
	}
	logline += ") ";
}
