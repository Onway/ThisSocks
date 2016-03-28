#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <unistd.h>

class EncryptBase
{
public:
	EncryptBase() : fd(-1) {}
    virtual ~EncryptBase() {}
    virtual bool SetServerFd(int fd) = 0;
    virtual bool SetClientFd(int fd) = 0;
    virtual ssize_t Read(void *buf, size_t len) = 0;
    virtual ssize_t Write(const void *buf, size_t len) = 0;
    int GetFd() { return fd; }
protected:
    int fd;
};

class EncryptFactory
{
public:
    EncryptBase* GetEncrypter();
};

class SimpleEncrypter : public EncryptBase
{
public:
    virtual bool SetServerFd(int fd);
    virtual bool SetClientFd(int fd);
    virtual ssize_t Read(void *buf, size_t len);
    virtual ssize_t Write(const void *buf, size_t len);
private:
    char randChar;
};

extern EncryptFactory GEncryptFactory;

#endif // ENCRYPT_H
