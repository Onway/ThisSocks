#include "Encrypt.h"
#include "Logger.h"
#include "Config.h"
#include <ctime>
#include <cstdlib>

#include <cryptopp/filters.h>  
#include <cryptopp/modes.h>  
#include <cryptopp/pwdbased.h>
#include <cryptopp/sha.h>

using std::string;

// global EncryptFactory
EncryptFactory GEncryptFactory;

// EncryptBase implementation
EncryptBase::EncryptBase() : fd(-1)
{
}

EncryptBase::~EncryptBase()
{
}

int EncryptBase::GetFd() const
{
	return fd;
}

// SimpleEncrypter implementation
SimpleEncrypter::SimpleEncrypter() : randChar(-1)
{
}

bool SimpleEncrypter::SetClientFd(int fd)
{
    this->fd = fd;
    char buf[1];
    if (read(this->fd, buf, 1) != 1) {
        GLogger.LogErr(LOG_DEBUG, "SetClientFd error");
        return false;
    }

    this->randChar = buf[0];
    return true;
}

bool SimpleEncrypter::SetServerFd(int fd)
{
    this->fd = fd;
    srandom(time(NULL));
    this->randChar = random() % 128;
    if (write(this->fd, &this->randChar, 1) != 1) {
        GLogger.LogErr(LOG_DEBUG, "SetServerFd error");
        return false;
    }
    return true;
}

ssize_t SimpleEncrypter::Read(void *buf, size_t len) const
{
    char *rbuf = (char *)buf;
    ssize_t readn = read(this->fd, buf, len);
    if (readn > 0) {
        for (ssize_t i = 0; i < readn; ++i) {
            rbuf[i] -= this->randChar;
        }
    }
    return readn;
}

ssize_t SimpleEncrypter::Write(const void *buf, size_t len) const
{
    char *wbuf = new char[len]();
    char *rbuf = (char *)buf;
    for (size_t i = 0; i < len; ++i) {
        wbuf[i] = rbuf[i] + this->randChar;
    }
    ssize_t ret = write(this->fd, wbuf, len);
    delete[] wbuf;
    return ret;
}

SimpleEncrypter* SimpleEncrypter::clone() const
{
	return new SimpleEncrypter(*this);
}

// Aes128Ecb implementation
bool Aes128Ecb::SetClientFd(int fd)
{
    this->fd = fd;
	char buf[256];
	string user = GConfig.Username;
	string pwd = GConfig.Password;

	buf[0] = user.size();
	strncpy(buf + 1, user.c_str(), buf[0]);
    if (write(this->fd, buf, buf[0] + 1) != buf[0] + 1) {
        GLogger.LogErr(LOG_DEBUG, "SetClientFd error");
        return false;
    }

	InitAes(pwd);
    return true;
}

bool Aes128Ecb::SetServerFd(int fd)
{
	char buf[256];
	int readn = 0;
    if ((readn = read(this->fd, buf, sizeof(buf))) <= 0) {
        GLogger.LogErr(LOG_DEBUG, "SetServerFd error");
        return false;
	}

	if (buf[0] + 1 != readn) {
		GLogger.LogMsg(LOG_DEBUG, "SetServerFd length invalid");
		return false;
	}

	string user(buf + 1, buf[0]);
    return true;
}

ssize_t Aes128Ecb::Read(void *buf, size_t len) const
{
	return 0;
	/*
    char *rbuf = (char *)buf;
    ssize_t readn = read(this->fd, buf, len);
    if (readn > 0) {
        for (ssize_t i = 0; i < readn; ++i) {
            rbuf[i] -= this->randChar;
        }
    }
    return readn;
	*/
}

ssize_t Aes128Ecb::Write(const void *buf, size_t len) const
{
	return 0;
	/*
    char *wbuf = new char[len]();
    char *rbuf = (char *)buf;
    for (size_t i = 0; i < len; ++i) {
        wbuf[i] = rbuf[i] + this->randChar;
    }
    ssize_t ret = write(this->fd, wbuf, len);
    delete[] wbuf;
    return ret;
	*/
}

Aes128Ecb* Aes128Ecb::clone() const
{
	return new Aes128Ecb(*this);
}

void Aes128Ecb::InitAes(string pwd)
{
	CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);
	CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> kdf;
	kdf.DeriveKey(key.data(), key.size(), 0,
			(byte*)pwd.data(), pwd.size(), NULL, 0, iterations);

    e.SetKey(key, key.size());
	d.SetKey(key, key.size());
}

// EncryptFactory implementation
EncryptBase* EncryptFactory::GetEncrypter()
{
    return new SimpleEncrypter();
}
