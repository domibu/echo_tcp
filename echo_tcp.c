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

static struct uloop_fd server;
static const char *port = "10000";

static void server_cb(struct uloop_fd *fd, unsigned int events)
{
	unsigned int sl = sizeof(struct sockaddr_in);
	int sfd, n;
	
	struct sockaddr_in client_addr;
	char buffer[256], *ip;

	sfd = accept(server.fd, (struct sockaddr *) &client_addr, &sl);
	if (sfd < 0) {
		fprintf(stderr, "Accept failed\n");
		return;
	}

	
	n = read( sfd, buffer, 255);
	if (n < 0) perror("Error reading from socket");
	else 
	{
		ip = inet_ntoa(client_addr.sin_addr);
		printf("IP: %s	Message: %s\n", ip, buffer);
		n = write( sfd, buffer, n);
		if (n < 0) perror("Error writing to socket");
	}
	close(sfd);
}

static int run_server(void)
{

	server.cb = server_cb;
	server.fd = usock(USOCK_TCP | USOCK_SERVER | USOCK_IPV4ONLY | USOCK_NUMERIC, INADDR_ANY, port);
	if (server.fd < 0) {
		perror("usock");
		return 1;
	}

	uloop_init();
	uloop_fd_add(&server, ULOOP_READ);
	uloop_run();

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
