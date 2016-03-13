#include "TcpServer.h"
#include "Config.h"
#include "Proxy.h"
#include "Logger.h"
#include <string>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

static void daemonize();

int main(int argc, char *argv[])
{
	int ret = GConfig.Init(argc, argv);
	if (ret <= 0) {
		return ret;
	}

	if (GConfig.RunAsDaemon) {
		daemonize();
	}
	GLogger.Init(GConfig.RunAsDaemon ? argv[0] : 0);

	string listenAddr;
	int listenPort;
	if (GConfig.RunAsClient) {
		listenAddr = GConfig.LocalAddress;
		listenPort = GConfig.LocalPort;
	} else {
		listenAddr = GConfig.ServerAddress;
		listenPort = GConfig.ServerPort;
		GPasswd.LoadFile(GConfig.PwdFile);
	}

    TcpServer srv;
    if (!srv.Init(listenAddr, listenPort)) {
		return -1;
	}
	srv.Run();

	// Proxy *proxy = new Proxy();
    // srv.Run(proxy);
	return -1; // error occured
}

void daemonize()
{
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
		perror("getrlimit error");
		exit(-1);
	}

	pid_t pid;
	if ((pid = fork()) < 0) {
		perror("fork error");
		exit(-1);
	} else if (pid != 0) {
		exit(0);
	}
	setsid();

	if (rl.rlim_max == RLIM_INFINITY) {
		rl.rlim_max = 1024;
	}
    for (unsigned i = 0; i < rl.rlim_max; ++i) {
		close(i);
	}

	int fd0, fd1, fd2;
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(fd0);
	fd2 = dup(fd0);
    assert(fd0 == 0);
    assert(fd1 == 1);
    assert(fd2 == 2);
}
