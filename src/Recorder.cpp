#include "Recorder.h"
#include "Logger.h"
#include "Config.h"
#include <cstring>
#include <cstdlib>

struct sockaddr_in Recorder::udpaddr;

pthread_key_t Recorder::pkey;

pthread_once_t Recorder::once = PTHREAD_ONCE_INIT;

void Recorder::CreateKey()
{
	if (!IsNeedRecord()) {
		return;
	}

	pthread_once(&once, InitThread);
	RecordSTime();
}

void Recorder::InitThread()
{
	pthread_key_create(&pkey, DeleteKey);

	memset(&udpaddr, 0, sizeof(udpaddr));
	udpaddr.sin_family = AF_INET;
	udpaddr.sin_port = htons(GConfig.RecordPort);
	if (inet_pton(AF_INET, GConfig.RecordAddress.c_str(), &udpaddr.sin_addr) <= 0) {
		GLogger.LogErr(LOG_ERR, "inet_pton RecordAddress error");
		exit(-1);
	}
}

void Recorder::DeleteKey(void* arg)
{
	if (arg != NULL) {
		RecordInfo* info = (RecordInfo*)arg;
		RecordETime(info);
		// info->Print();
		info->Print(udpaddr);

		delete info;
		info = NULL;
		arg = NULL;
	}
}

RecordInfo* Recorder::GetRecordInfo()
{
	RecordInfo* info = (RecordInfo*)pthread_getspecific(pkey);
	if (info == NULL) {
		info = new RecordInfo();
		pthread_setspecific(pkey, info);
	}
	return info;
}

bool Recorder::IsNeedRecord()
{
	return !GConfig.RecordAddress.empty() && GConfig.RecordPort != 0;
}

void Recorder::RecordUser(std::string user)
{
	if (!IsNeedRecord()) {
		return;
	}

	RecordInfo* info = GetRecordInfo();
	info->User = user;
}

void Recorder::RecordAddress(unsigned int ip, unsigned short port)
{
	if (!IsNeedRecord()) {
		return;
	}

	RecordInfo* info = GetRecordInfo();
	info->IP = ip;
	info->Port = port;
}

void Recorder::RecordUpload(unsigned int size)
{
	if (!IsNeedRecord()) {
		return;
	}

	RecordInfo* info = GetRecordInfo();
	info->Upload += size;
}

void Recorder::RecordDownload(unsigned int size)
{
	if (!IsNeedRecord()) {
		return;
	}

	RecordInfo* info = GetRecordInfo();
	info->Download += size;
}

void Recorder::RecordSTime()
{
	RecordInfo* info = GetRecordInfo();
	gettimeofday(&info->STime, NULL);
}

void Recorder::RecordETime(RecordInfo* info)
{
	gettimeofday(&info->ETime, NULL);
}

RecordInfo::RecordInfo()
	: IP(0), Port(0), Upload(0), Download(0)
{
}

void RecordInfo::Print()
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

void RecordInfo::Print(const struct sockaddr_in& udpaddr)
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

size_t RecordInfo::ConvertToBytes(unsigned char* buf)
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

size_t RecordInfo::UShortToBytes(unsigned char* buf, unsigned short val)
{
	memcpy(buf, &val, sizeof(val));
	return sizeof(val);
}

size_t RecordInfo::UIntToBytes(unsigned char* buf, unsigned int val)
{
	memcpy(buf, &val, sizeof(val));
	return sizeof(val);
}
