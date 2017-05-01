#ifndef RECORDER_H
#define RECORDER_H

#include <string>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

class RecordInfo
{
public:
	std::string User;
    std::string Host;
	struct timeval STime;
	struct timeval ETime;
	unsigned int IP;
	unsigned int Port;
	unsigned int Upload;
	unsigned int Download;

	RecordInfo();
	void Print();
	void Print(const struct sockaddr_in& udpaddr);

private:
	size_t ConvertToBytes(unsigned char* buf);
	size_t UShortToBytes(unsigned char* buf, unsigned short val);
	size_t UIntToBytes(unsigned char* buf, unsigned int val);
};

class Recorder
{
public:
	static void CreateKey();
	static void RecordUser(const std::string& user);
    static void RecordHost(const std::string host);
	static void RecordAddress(unsigned int ip, unsigned short port);
	static void RecordUpload(unsigned int size);
	static void RecordDownload(unsigned int size);

private:
	static void InitThread();
	static RecordInfo* GetRecordInfo();
	static bool IsNeedRecord();
	static void DeleteKey(void* arg);
	static void RecordSTime();
	static void RecordETime(RecordInfo* info);

	static pthread_key_t pkey;
	static pthread_once_t once;
	static struct sockaddr_in udpaddr;
};

#endif // RECORDER_H
