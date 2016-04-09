#include "Encrypt.h"
#include "Logger.h"
#include "Config.h"
#include "Passwd.h"
#include <ctime>
#include <cstdlib>
#include <math.h>

#include <cryptopp/aes.h>  
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

ssize_t SimpleEncrypter::Read(void *buf, size_t len)
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

ssize_t SimpleEncrypter::Write(const void *buf, size_t len)
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
	string pwd = GPasswd.GetPassword(user);
	if (pwd.size() == 0) {
		GLogger.LogMsg(LOG_DEBUG, "invalid username");
		return false;
	}

	InitAes(pwd);
    return true;
}

ssize_t Aes128Ecb::Read(void *buf, size_t len)
{
	char clen[2];
	ssize_t readn = 0;
	if ((readn = read(this->fd, clen, sizeof(clen))) <= 0) {
		return readn;
	} else if (readn != sizeof(clen)) {
		return -1;
	}
	short slen = *(short*)clen;

	char *rbuf = new char[slen];
	if (read(this->fd, rbuf, slen) != slen) {
		return -1;
	}

	string plain;
	CryptoPP::StringSource ss1((const byte*)rbuf, slen, true, 
        new CryptoPP::StreamTransformationFilter(d,
            new CryptoPP::StringSink(plain)
        ) 
    ); 

	size_t rlen = std::min(plain.size(), len);
	strncpy((char*)buf, plain.data(), rlen);
	delete[] rbuf;
	return rlen;
}

ssize_t Aes128Ecb::Write(const void *buf, size_t len)
{
	string cipher;
	CryptoPP::StringSource ss((const byte*)buf, len, true, 
        new CryptoPP::StreamTransformationFilter(e,
            new CryptoPP::StringSink(cipher)
        ) 
    ); 

	short slen = (short)cipher.size();
	char* wbuf = new char[slen + 2]();
	strncpy(wbuf, (const char*)&slen, 2);
	strncpy(wbuf + 2, cipher.data(), slen);

    ssize_t ret = write(this->fd, wbuf, slen + 2);
    delete[] wbuf;
	if (ret != 0) {
		return ret == slen + 2 ? len : -1;
	}
	return 0;
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
