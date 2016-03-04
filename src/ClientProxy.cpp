#include "ClientProxy.h"
#include "Logger.h"

void ClientProxy::Run(int srcfd, const char *request, int len)
{
    int tarfd = encrypter->GetFd();
    if (len != encrypter->Write(request, len)) {
        GLogger.LogErr(LOG_ERR, "write request to server error");
        return;
    }
    ForwardData(srcfd, tarfd);
}
