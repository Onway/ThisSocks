#include "Proxy.h"
#include "SocksProxy.h"
#include "HttpsProxy.h"
#include "HttpProxy.h"
#include "ClientProxy.h"
#include "Config.h"
#include "Logger.h"
#include "Utils.h"
#include "Encrypt.h"
#include "Counter.h"

using namespace std;

Proxy::Proxy() : encrypter(NULL)
{
}

Proxy::Proxy(const Proxy& proxy) : encrypter(NULL)
{
	if (proxy.encrypter != NULL) {
		encrypter = proxy.encrypter->clone();
	}
}

Proxy::~Proxy()
{
	if (encrypter != NULL) {
		delete encrypter;
		encrypter = NULL;
	}
}

Proxy& Proxy::operator=(const Proxy& proxy)
{
	if (encrypter != NULL) {
		delete encrypter;
	}
	encrypter = proxy.encrypter->clone();
	return *this;
}

void Proxy::Run(int srcfd)
{
	Proxy* proxy = NULL;
	if (GConfig.RunAsClient) {
		proxy = new ClientProxy();
	} else {
		proxy = new ServerProxy();
	}
	proxy->Process(srcfd);
	delete proxy;
}

/*
 * 在srcfd和tarfd之间相互转发数据
 * srcfd是发起请求的客户端
 * tarfd是处理请求的服务端
 */
void Proxy::ForwardData(int srcfd, int tarfd) const
{
    bool sendClose = false;
	fd_set fdset;
	while (1) {
        FD_ZERO(&fdset);
        FD_SET(srcfd, &fdset);
        FD_SET(tarfd, &fdset);
		
        if (select(max(srcfd, tarfd) + 1, &fdset, NULL, NULL, NULL) < 0) {
			if (errno == EINTR) {
				continue;
			}
            GLogger.LogErr(LOG_ERR, "select error");
			break;
		}

		// 转发请求
        int ret;
        if (FD_ISSET(srcfd, &fdset)) {
            if ((ret = ForwardData(srcfd, tarfd, true)) < 0) { // error
                break;
            } else if (ret == 0 && sendClose) {
                break;
            } else if (ret == 0) { // 发送端已经关闭，触发接收端的半关闭
                sendClose = true;
				shutdown(tarfd, SHUT_WR);
            }
		}

		// 转发回复
        if (FD_ISSET(tarfd, &fdset)) {
            if ((ret = ForwardData(tarfd, srcfd, false)) < 0) {
                break;
            } else if (ret == 0 && sendClose) {
                break;
            } else if (ret == 0) { // 发送端已经关闭，触发接收端的半关闭
                sendClose = true;
				shutdown(srcfd, SHUT_WR);
            }
		}
	}
}

/*
 * 将来自srcfd的数据转发至tarfd
 * isRequest标识是否用户的请求数据
 * 成功返回1;失败返回-1;srcfd关闭返回0
 */
int Proxy::ForwardData(int srcfd, int tarfd, bool isRequest) const
{
	// 标识读端是否已加密
    bool encrypted = (GConfig.RunAsClient && !isRequest) // 客户端收到回复
        || (!GConfig.RunAsClient && isRequest); // 服务器收到请求

	// 读数据
    char buf[MAXBUF];
    int readcnt;
    if (encrypted) {
        readcnt = encrypter->Read(buf, MAXBUF);
    } else {
        readcnt = read(srcfd, buf, MAXBUF);
    }
    if (readcnt < 0) {
        GLogger.LogErr(LOG_NOTICE, "read forward data error from %s",
                isRequest ? "client" : "server");
        return -1;
    } else if (readcnt == 0) {
        return 0;
    }

	// 服务端收到回复，记录下载字节
	if (!GConfig.RunAsClient && !isRequest) {
		Counter::RecordDownload(readcnt);
	}

	// 写数据
    int writecnt;
    if (encrypted) { // 如果读端已经加密，则写端不加密，反之也是这样
        writecnt = write(tarfd, buf, readcnt);
    } else {
        writecnt = encrypter->Write(buf, readcnt);
    }
    if (writecnt != readcnt) {
        GLogger.LogErr(LOG_ERR, "write forward data to %s error",
				isRequest ? "server" : "client");
        return -1;
	}

	// 服务端转发请求，记录上传字节
	if (!GConfig.RunAsClient && isRequest) {
		Counter::RecordUpload(writecnt);
	}

    return 1;
}
