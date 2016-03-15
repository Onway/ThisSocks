#include "TcpServer.h"
#include "Logger.h"
#include "Config.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

using namespace std;

bool TcpServer::Init(string srvAddr, int port)
{
	if ((listenfd = CreateListenSocket(srvAddr, port)) < 0) {
		return false;
	}

	return true;
}

void TcpServer::Run()
{
	int connfd;

	while (1) {
		if ((connfd = accept(listenfd, NULL, 0)) == -1) {
			if (errno == EINTR || errno == ECONNABORTED) {
				continue;
			}
			GLogger.LogErr(LOG_ERR, "accept error");
			break;
		}

		if (!StartProcessThread(connfd)) {
			break;
		}
	}
}

int TcpServer::CreateListenSocket(string srvAddr, int port)
{
	int listenfd;
	struct sockaddr_in servaddr;

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if (inet_pton(AF_INET, srvAddr.c_str(), &servaddr.sin_addr) <= 0) {
		GLogger.LogErr(LOG_ERR, "inet_pton error");
		return -1;
	}

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		GLogger.LogErr(LOG_ERR, "socket error");
		return -1;
	}

	int flag = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1) {
		GLogger.LogErr(LOG_ERR, "setsockopt error");
		return -1;
	}

	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		GLogger.LogErr(LOG_ERR, "bind error");
		return -1;
	}

	if (listen(listenfd, 100) == -1) {
		GLogger.LogErr(LOG_ERR, "listen error");
		return -1;
	}

	return listenfd;
}

bool TcpServer::SetupSignalHandlers()
{
	struct sigaction act;

	act.sa_handler = sig_chld;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
#ifdef SA_RESTART
	act.sa_flags |= SA_RESTART;
#endif
	if (sigaction(SIGCHLD, &act, NULL) == -1) {
		GLogger.LogErr(LOG_ERR, "sigaction error");
		return false;
	}
	return true;
}

void TcpServer::sig_chld(int signo)
{
	if (signo == SIGCHLD) {
		while (waitpid(-1, NULL, WNOHANG) > 0)
			;
	}
}

bool TcpServer::StartProcessThread(int connfd)
{
	pthread_attr_t pattr;
	if (pthread_attr_init(&pattr) != 0) {
		GLogger.LogMsg(LOG_ERR, "pthread_attr_init error");
		return false;
	}
	if (pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_DETACHED) != 0) {
		GLogger.LogMsg(LOG_ERR, "pthread_attr_setdetachstate error");
		return false;
	}

	pthread_t tid;
	if (pthread_create(&tid, &pattr, ProcessRequestThread,
				(void *)(long)connfd) != 0) {
		GLogger.LogMsg(LOG_ERR, "pthread_create error");
		return false;
	}
	
	if (pthread_attr_destroy(&pattr) != 0) {
		GLogger.LogMsg(LOG_ERR, "pthread_attr_destroy error");
		return false;
	}
	return true;
}

void * TcpServer::ProcessRequestThread(void *arg)
{
	int connfd = (int)(long)arg;
	int srvfd = -1;
	Proxy proxy;

	proxy.Run(connfd, srvfd);
	close(connfd);
	if (srvfd != -1) {
		close(srvfd);
	}

	return (void *)0;
}
