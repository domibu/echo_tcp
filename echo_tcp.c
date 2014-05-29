#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "uloop.h"
#include "usock.h"
#include "list.h"

static struct uloop_fd server;
static const char *port = "10000";

static struct list_head childs = LIST_HEAD_INIT(childs);


static void child_end(struct uloop_process *uproc, int ret)
{
	fprintf(stderr, "child end: %d\n", uproc->pid);
	free(uproc);
}

static int child_dostuff(int sfd, struct sockaddr_in client_addr)
{
	char buffer[256], *ip;
	int n;
	pid_t pid;
	
	while ((n = read( sfd, buffer, 255)) > 0 ) {	
		pid = getpid();
		buffer[n] = '\0';
		ip = inet_ntoa(client_addr.sin_addr);
		fprintf(stderr, "IP: %s PID: %d	Message: %s\n", ip, pid, buffer);
		write( sfd, buffer, n);
	}
	return 0;
}


static void server_cb(struct uloop_fd *fd, unsigned int events)
{
	unsigned int sl = sizeof(struct sockaddr_in);
	int sfd, ret;
	pid_t pid, ppid;
	
	struct sockaddr_in client_addr;
	
	for (;;) {
		pid = getpid();
		ppid = getppid();
	
		sfd = accept(server.fd, (struct sockaddr *) &client_addr, &sl);
		if (sfd < 0) {
			return;
		}

	
		fprintf(stderr, "New conn PID: %d PPID: %d\n", pid, ppid);
		struct uloop_process *uproc = calloc(1, sizeof(*uproc));
		if ( !uproc || (uproc->pid = fork()) == -1) {
			free(uproc);
			close(sfd);	
		}
		if (uproc->pid != 0) {
			//parent
			uproc->cb = child_end;
			uloop_process_add(uproc);
			close(sfd);
		}
		else {	//child do stuff
			ret = child_dostuff(sfd, client_addr);
			close(sfd);
			exit(ret);
		}
	}
}


static int run_server(void)
{

	pid_t pid = getpid();
	pid_t ppid = getppid();
	fprintf(stderr, "main PPID: %d PID: %d\n", ppid, pid);

	server.cb = server_cb;
	server.fd = usock(USOCK_TCP | USOCK_SERVER | USOCK_IPV4ONLY | USOCK_NUMERIC, INADDR_ANY, port);
	if (server.fd < 0) {
		perror("usock");
		return 1;
	}

	uloop_init();
	uloop_fd_add(&server, ULOOP_READ);
	uloop_run();
	
	uloop_done();
	return 0;
}

static int usage(const char *name)
{
	fprintf(stderr, "Usage: %s -p <port>\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	int ch;

	while ((ch = getopt(argc, argv, "p:")) != -1) {
		switch(ch) {
		case 'p':
			port = optarg;
			break;
		default:
			return usage(argv[0]);
		}
	}

	return run_server();
}
