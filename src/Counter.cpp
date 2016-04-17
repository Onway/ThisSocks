#include "Counter.h"
#include "Logger.h"

pthread_key_t Counter::pkey;

pthread_once_t Counter::once = PTHREAD_ONCE_INIT;

void Counter::CreateKey()
{
	pthread_once(&once, InitThread);
}

void Counter::InitThread()
{
	pthread_key_create(&pkey, DeleteKey);
}

void Counter::DeleteKey(void* arg)
{
	if (arg != NULL) {
		ThreadInfo* info = (ThreadInfo*)arg;
		info->Print();
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

void Counter::RecordUser(std::string user)
{
	ThreadInfo* info = GetThreadInfo();
	info->User = user;
}

void Counter::RecordAddress(unsigned int ip, unsigned short port)
{
	ThreadInfo* info = GetThreadInfo();
	info->IP = ip;
	info->Port = port;
}

void Counter::RecordUpload(unsigned int size)
{
	ThreadInfo* info = GetThreadInfo();
	info->Upload += size;
}

void Counter::RecordDownload(unsigned int size)
{
	ThreadInfo* info = GetThreadInfo();
	info->Download += size;
}

ThreadInfo::ThreadInfo()
	: IP(0), Port(0), Upload(0), Download(0)
{
}

void ThreadInfo::Print()
{
	GLogger.LogMsg(
			LOG_DEBUG,
			"user %s from %u:%u upload %u bytes, donwload %u bytes",
			User.c_str(), IP, Port, Upload, Download);
}
