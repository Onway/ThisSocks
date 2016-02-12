#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define MAXBUF 4096
#define BACKLOG 100

#define MAX(a, b) (a > b ? a : b)

void msg_quit(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fprintf(stdout, "\n");
	exit(1);
}

void err_quit(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, " %s\n", strerror(errno));
	exit(1);
}

void sigchld_handler(int signo)
{
	if (signo == SIGCHLD) {
		while (waitpid(-1, NULL, WNOHANG) > 0)
			;
	}
}

ssize_t readn(int fd, void *buf, size_t count)
{
	size_t nleft;
	ssize_t nread;
	char *sptr;

	sptr = (char *)buf;
	nleft = count;
	while (nleft > 0) {
		if ((nread = read(fd, sptr, nleft)) < 0) {
			if (errno == EINTR) {
				nread = 0;
			}
			else {
				return -1;
			}
		} else if (nread == 0) {
			break;
		}
		
		nleft -= nread;
		sptr += nread;
	}
	return count - nleft;
}

void process_auth(int clientfd)
{
	char buf[3];
	if (3 != readn(clientfd, buf, 3)) {
		err_quit("[auth] read error:");
	}

	char expect[3] = { 5, 1, 0 };
	if (0 != bcmp(buf, expect, 3)) {
		msg_quit("[auth] auth error");
	}

	char res[2] = { 5, 0 };
	if (2 != write(clientfd, res, 2)) {
		err_quit("[auth] write error:");
	}
}

int process_connect(int clientfd)
{
	char buf[4];
	uint32_t ip; 
	if (4 != readn(clientfd, buf, 4)) {
		err_quit("[connect] read init error:");
	}

	char expect[4] = { 5, 1, 0 };
	if (0 != bcmp(buf, expect, 3)) {
		msg_quit("[connect] init error");
	}
	if (buf[3] == 1) {
		if (4 != readn(clientfd, buf, 4)) {
			err_quit("[connect] read ip error 2:");
		}
		ip = *((uint32_t *)buf);
	} else if (buf[3] == 3) {
		if (1 != readn(clientfd, buf, 1)) {
			err_quit("[connect] read host name length error:");
		}
		int len = buf[0];
		if (len != readn(clientfd, buf, len)) {
			err_quit("[connect] read host name error:");
		}
		buf[len] = 0;
		printf("%s\n", buf);
		struct hostent *hptr = gethostbyname(buf);
		if (hptr == NULL) {
			err_quit("[connect] gethostbyname error:");
		}
		ip = *((uint32_t *)*hptr->h_addr_list);
	}


	if (2 != readn(clientfd, buf, 2)) {
		err_quit("[connect] read port error:");
	}
	uint16_t port = *((uint16_t *)buf);

	struct sockaddr_in remoteaddr;
	bzero(&remoteaddr, sizeof(remoteaddr));
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = ip;
	remoteaddr.sin_port = port;
	
	int remotefd = socket(AF_INET, SOCK_STREAM, 0);
	if (remotefd == -1) {
		err_quit("[connect] socket error:");
	}

	if (connect(remotefd,
			(struct sockaddr *)&remoteaddr,
		   	sizeof(remoteaddr)) < 0) {
		err_quit("[connect] connect error:");
	}

	char res[10];
	bzero(res, 10);
	res[0] = 5;
	res[3] = 1;
	if (10 != write(clientfd, res, 10)) {
		err_quit("[connect] write error:");
	}

	return remotefd;
}

int process_forward(int clientfd, int remotefd)
{
	char buf[MAXBUF];
	fd_set fdset;
	while (1) {
		FD_ZERO(&fdset);
		FD_SET(clientfd, &fdset);
		FD_SET(remotefd, &fdset);
		
		if (select(MAX(clientfd, remotefd) + 1, &fdset, NULL, NULL, NULL) < 0) {
			err_quit("[forward] select error:");
		}

		if (FD_ISSET(clientfd, &fdset)) {
			int readcnt = read(clientfd, buf, MAXBUF);
			if (readcnt < 0) {
				err_quit("[forward] read error from client:");
			}
			else if (readcnt == 0) {
				msg_quit("[forward] read end from client");
			}

			if (write(remotefd, buf, readcnt) <= 0) {
				err_quit("forward] write to remote error:");
			}
		}

		if (FD_ISSET(remotefd, &fdset)) {
			int readcnt = read(remotefd, buf, MAXBUF);
			if (readcnt < 0) {
				err_quit("[forward] read error from remote:");
			}
			else if (readcnt == 0) {
				msg_quit("[forward] read end from remote");
			}

			if (write(clientfd, buf, readcnt) <= 0) {
				err_quit("forward] write to client error:");
			}
		}
	}

	return 0;
}

int setup_server()
{
	int listenfd;
	struct sockaddr_in servaddr;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(1081);
	if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0) {
		err_quit("inet_pton error:");
	}

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		err_quit("socket error:");
	}

	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		err_quit("bind error:");
	}

	if (listen(listenfd, BACKLOG) == -1) {
		err_quit("listen error:");
	}

	return listenfd;
}

void run_child(int clientfd)
{
	process_auth(clientfd);
	int remotefd = process_connect(clientfd);
	process_forward(clientfd, remotefd);
}

int main(int argc, char *argv[])
{
	pid_t cid;
	int listenfd, clientfd;

	if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
		err_quit("signal error:");
	}

   	listenfd = setup_server();
	while (1) {
		if ((clientfd = accept(listenfd, NULL, 0)) == -1) {
			err_quit("accept error:");
		}

		if ((cid = fork()) < 0) {
			err_quit("fork error:");
		} else if (cid == 0) {
			close(listenfd);
			run_child(clientfd);
		}
		
		close(clientfd);
	}

	return 0;
}
