#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdint.h>
#include "connection.h"

int listener_init(ConnectionListener *sock, int port) {
	if (!sock || port < 1 || port > 65535) {
		errno = EINVAL;
		return -1;
	}

	//create server socket
	sock->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock->fd < 0) {
		fprintf(stderr, "Failed to create socket\n");
		return -1;
	}

	//set socket options
	int opt = 1;
	if ((setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) < 0) {
		fprintf(stderr, "Failed to set socket options\n");
		close(sock->fd);
		return -1;
	}

	//create sockaddr_in containing server info
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(sock->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "Failed to bind socket\n");
		close(sock->fd);
		return -1;
	}

	if (listen(sock->fd, 10) < 0) {
		fprintf(stderr, "Failed to listen on socket\n");
		close(sock->fd);
		return -1;
	}
	
	return 0;
}

int listener_accept(ConnectionListener *sock) {
	if (!sock || sock->fd < 0 || sock->fd > 65535) {
		errno = EINVAL;
		return -1;
	}

	//connect to a client
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(sock->fd, (struct sockaddr *)&client_addr, &client_len);

	if (client_fd < 0) {
		fprintf(stderr, "Failed to accept connection\n");
		return -1;
	}

	struct timeval timeout = {5, 0};
	if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		fprintf(stderr, "Failed to set timeout on socket\n");
		close(client_fd);
		return -1;
	}

	return client_fd;
}
