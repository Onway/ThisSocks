#include "Encrypt.h"
#include "Logger.h"
#include <ctime>
#include <cstdlib>

EncryptFactory GEncryptFactory;

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
    char *wbuf = new char[len];
    char *rbuf = (char *)buf;
    for (size_t i = 0; i < len; ++i) {
        wbuf[i] = rbuf[i] + this->randChar;
    }
    ssize_t ret = write(this->fd, wbuf, len);
    delete wbuf;
    return ret;
}

EncryptBase* EncryptFactory::GetEncrypter()
{
    return new SimpleEncrypter();
}
