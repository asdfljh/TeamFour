#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define PORT_NUMBER 8001
#define BUF_SIZE 65536

uint32_t change_endian(uint32_t src);
int check_ip_arg(char* start_ip, char* end_ip);

int port_bind(char* launcher_ip);

int main(int argc, char** argv) {
	int r;
	int server_socket;
	int client_socket;
	int client_address_size;
	struct sockaddr_in client_address;
	char buffer[BUF_SIZE];

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <launcher_ip> <start_ip> <end_ip>\n", argv[0]);
		exit(1);
	}

	if (check_ip_arg(argv[2], argv[3]) == -1) {
		fprintf(stderr, "IP Argument Error\n");
		exit(1);
	}

	server_socket = port_bind(argv[1]);

	if (server_socket == -1) {
		exit(1);
	}

	while(1) {
		client_address_size = sizeof(client_address);
		client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_size);

		if (client_socket == -1) {
			fprintf(stderr, "accept() error\n");
			exit(1);
		}

		printf("Hello %s\n", inet_ntoa(client_address.sin_addr));

		while(1) {
			memset(buffer, 0, BUF_SIZE);
			memset(filePath, 0, FILE_NAME_SIZE);

			r = read(client_socket, buffer, BUF_SIZE);

			if (r <= 0) {
				break;
			}

			printf("%s, %d\n", buffer, r);

		}

		close(client_socket);

	}
	close(server_socket);

	return 0;

}

int port_bind(char* launcher_ip) {
	int server_socket;
	struct sockaddr_in server_address;

	server_socket = socket(PF_INET, SOCK_STREAM, 0);

	if (server_socket == -1) {
		fprintf(stderr, "socket() error\n");
		return -1;
	}

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(launcher_ip);
	server_address.sin_port = htons(PORT_NUMBER);

	if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
		fprintf(stderr, "bind() error\n");
		close(server_socket);
		return -1;
	}

	if (listen(server_socket, 5) == -1) {
		fprintf(stderr, "listen() error\n");
		close(server_socket);
		return -1;
	}

	return server_socket;
}


uint32_t change_endian(uint32_t src) {
	uint32_t a, b, c, d, result;

	a = (src >> 24) & 0xFF;
	b = ((src >> 16) & 0xFF) << 8;
	c = ((src >> 8) & 0xFF) << 16;
	d = (src & 0xFF) << 24;
	result = a | b | c | d;

	return result;
}

int check_ip_arg(char* start_ip, char* end_ip) {
	uint32_t start = change_endian(inet_addr(start_ip));
	uint32_t end = change_endian(inet_addr(end_ip));

	if (start <= end) {
		return 0;
	}

	return -1;
}


