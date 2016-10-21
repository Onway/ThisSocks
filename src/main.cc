#include "tcp_server.h"
#include "config.h"
#include "logger.h"
#include "passwd.h"
#include <string>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LOCKDIR "/var/tmp"

using std::string;

static void daemonize();
static void savepid();
static void dropprivilege();
static string getlockfile();
static void setupsignal();
static void sig_handler(int signo);

int main(int argc, char *argv[])
{
	dropprivilege();

	int ret = GConfig.Init(argc, argv);
	if (ret <= 0) {
		return ret;
	}

	if (GConfig.RunAsDaemon) {
		daemonize();
	}
	GLogger.Init(GConfig.RunAsDaemon ? argv[0] : 0);
	setupsignal();
	if (GConfig.RunAsDaemon) {
		savepid();
	}

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
	srv.Run(listenAddr, listenPort);

	return 1; // error occured
}

void daemonize()
{
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
		perror("getrlimit error");
		exit(1);
	}

	pid_t pid;
	if ((pid = fork()) < 0) {
		perror("fork error");
		exit(1);
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

void savepid()
{
	string lockfile = getlockfile();
	int fd = open(lockfile.c_str(), O_RDWR | O_CREAT, 
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		GLogger.LogErr(LOG_ERR, "open lockfile error");
		exit(1);
	}

	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	if (fcntl(fd, F_SETLK, &fl) < 0) {
		if (errno == EACCES || errno == EAGAIN) {
			exit(1); // already running
		}
		GLogger.LogErr(LOG_ERR, "fcntl lockfile error");
		exit(1);
	}

	char buf[16];
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf));
}

void dropprivilege()
{
	struct passwd *pwd = getpwnam("root");
	if (pwd == NULL) {
		perror("getpwname root error");
		exit(1);		
	}

	uid_t euid = geteuid();
	if (euid != pwd->pw_uid) {
		return;
	}

	pwd = getpwnam("nobody");
	if (pwd == NULL) {
		perror("getpwname nobody error");
		exit(1);
	}
	if (setgid(pwd->pw_gid) == -1) {
		perror("setgid nodoby error");
		exit(1);
	}
	if (setuid(pwd->pw_uid) == -1) {
		perror("setuid nodoby error");
		exit(1);
	}
}

void setupsignal()
{
	struct sigaction act;
	act.sa_handler = sig_handler;	
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if (sigaction(SIGINT, &act, NULL) == -1
		|| sigaction(SIGQUIT, &act, NULL) == -1
		|| sigaction(SIGTERM, &act, NULL) == -1) {
		GLogger.LogErr(LOG_ERR, "sigaction error");		
		exit(1);
	}
}

void sig_handler(int signo)
{
	if (signo == SIGINT || signo == SIGQUIT
		|| signo == SIGTERM) {
		string lockfile = getlockfile();
		unlink(lockfile.c_str());
		exit(0);
	}
}

string getlockfile()
{
	string lockfile;
	if (GConfig.RunAsClient) {
		lockfile = LOCKDIR + string("/ThisSocks_C.pid");
	} else {
		lockfile = LOCKDIR + string("/ThisSocks_S.pid");
	}
	return lockfile;
}
