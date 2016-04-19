#include "Counter.h"
#include "Logger.h"
#include "Config.h"
#include <cstring>
#include <cstdlib>

struct sockaddr_in Counter::udpaddr;

pthread_key_t Counter::pkey;

pthread_once_t Counter::once = PTHREAD_ONCE_INIT;

void Counter::CreateKey()
{
	if (!IsNeedRecord()) {
		return;
	}

	pthread_once(&once, InitThread);
	RecordSTime();
}

void Counter::InitThread()
{
	pthread_key_create(&pkey, DeleteKey);

	memset(&udpaddr, 0, sizeof(udpaddr));
	udpaddr.sin_family = AF_INET;
	udpaddr.sin_port = htons(GConfig.StatPort);
	if (inet_pton(AF_INET, GConfig.StatAddress.c_str(), &udpaddr.sin_addr) <= 0) {
		GLogger.LogErr(LOG_ERR, "inet_pton StatAddress error");
		exit(-1);
	}
}

void Counter::DeleteKey(void* arg)
{
	if (arg != NULL) {
		ThreadInfo* info = (ThreadInfo*)arg;
		RecordETime(info);
		info->Print();
		info->Print(udpaddr);

		delete info;
		info = NULL;
		arg = NULL;
	}
}

ThreadInfo* Counter::GetThreadInfo()
{
	ThreadInfo* info = (ThreadInfo*)pthread_getspecific(pkey);
	if (info == NULL) {
		info = new ThreadInfo();
		pthread_setspecific(pkey, info);
	}
	return info;
}

bool Counter::IsNeedRecord()
{
	return !GConfig.StatAddress.empty() && GConfig.StatPort != 0;
}

void Counter::RecordUser(std::string user)
{
	if (!IsNeedRecord()) {
		return;
	}

	ThreadInfo* info = GetThreadInfo();
	info->User = user;
}

void Counter::RecordAddress(unsigned int ip, unsigned short port)
{
	if (!IsNeedRecord()) {
		return;
	}

	ThreadInfo* info = GetThreadInfo();
	info->IP = ip;
	info->Port = port;
}

void Counter::RecordUpload(unsigned int size)
{
	if (!IsNeedRecord()) {
		return;
	}

	ThreadInfo* info = GetThreadInfo();
	info->Upload += size;
}

void Counter::RecordDownload(unsigned int size)
{
	if (!IsNeedRecord()) {
		return;
	}

	ThreadInfo* info = GetThreadInfo();
	info->Download += size;
}

void Counter::RecordSTime()
{
	ThreadInfo* info = GetThreadInfo();
	gettimeofday(&info->STime, NULL);
}

void Counter::RecordETime(ThreadInfo* info)
{
	gettimeofday(&info->ETime, NULL);
}

ThreadInfo::ThreadInfo()
	: IP(0), Port(0), Upload(0), Download(0)
{
}

void ThreadInfo::Print()
{
	GLogger.LogMsg(
			LOG_DEBUG,
			"\nSTime: %ld.%ld"
			"\nETime: %ld.%ld"
			"\nUser: %s"
		   	"\nConnect: %u,%u"
		    "\nUpload: %u"
		   	"\nDonwload: %u"
			"\n",
			STime.tv_sec, STime.tv_usec,
			ETime.tv_sec, ETime.tv_usec,
			User.c_str(),
			IP, Port,
			Upload, Download);
}

void ThreadInfo::Print(const struct sockaddr_in& udpaddr)
{
	unsigned char buf[512];
	size_t len = ConvertToBytes(buf);

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		GLogger.LogErr(LOG_NOTICE, "udp socket error");
		return;
	}

	sendto(sockfd, buf, len, 0,
			(struct sockaddr*)&udpaddr, sizeof(udpaddr));
}

size_t ThreadInfo::ConvertToBytes(unsigned char* buf)
{
	size_t offset = 0;

	offset += UShortToBytes(buf + offset, htons(1)); // version
	offset += UShortToBytes(buf + offset, htons(32 + User.size())); // length
	offset += UIntToBytes(buf + offset, htonl(STime.tv_sec));
	offset += UIntToBytes(buf + offset, htonl(STime.tv_usec));
	offset += UIntToBytes(buf + offset, htonl(ETime.tv_sec));
	offset += UIntToBytes(buf + offset, htonl(ETime.tv_usec));
	offset += UIntToBytes(buf + offset, htonl(IP));
	offset += UIntToBytes(buf + offset, htonl(Port));
	offset += UIntToBytes(buf + offset, htonl(Upload));
	offset += UIntToBytes(buf + offset, htonl(Download));
	memcpy(buf + offset, User.data(), User.size());
	offset += User.size();

	return offset;
}

size_t ThreadInfo::UShortToBytes(unsigned char* buf, unsigned short val)
{
	memcpy(buf, &val, sizeof(val));
	return sizeof(val);
}

size_t ThreadInfo::UIntToBytes(unsigned char* buf, unsigned int val)
{
	memcpy(buf, &val, sizeof(val));
	return sizeof(val);
}
