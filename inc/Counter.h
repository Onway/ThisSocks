#ifndef COUNTER_H
#define COUNTER_H

#include <string>
#include <pthread.h>

class Counter
{
public:
	static void CreateKey();
	static void RecordUser(std::string user);
	static void RecordAddress(unsigned int ip, unsigned short port);
	static void RecordUpload(unsigned int size);
	static void RecordDownload(unsigned int size);

private:
	static pthread_key_t pkey;
	static pthread_once_t once;
};

#endif // COUNTER_H
