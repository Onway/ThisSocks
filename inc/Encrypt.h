#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <unistd.h>

class EncryptBase
{
public:
	EncryptBase();
    virtual ~EncryptBase();
    virtual bool SetServerFd(int fd) = 0;
    virtual bool SetClientFd(int fd) = 0;
    virtual ssize_t Read(void *buf, size_t len) const = 0;
    virtual ssize_t Write(const void *buf, size_t len) const = 0;
	virtual EncryptBase* clone() const = 0;
    int GetFd() const;

protected:
    int fd;
};

class SimpleEncrypter : public EncryptBase
{
public:
	SimpleEncrypter();
    bool SetServerFd(int fd);
    bool SetClientFd(int fd);
    ssize_t Read(void *buf, size_t len) const;
    ssize_t Write(const void *buf, size_t len) const;
	SimpleEncrypter* clone() const;

private:
    char randChar;
};

class EncryptFactory
{
public:
    EncryptBase* GetEncrypter();
};

extern EncryptFactory GEncryptFactory;

#endif // ENCRYPT_H
