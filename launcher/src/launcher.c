#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define PORT_NUMBER 8001
#define BUF_SIZE 65536

void main(int argc, char** argv) {
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
}
