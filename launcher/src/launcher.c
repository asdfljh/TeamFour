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

	if (check_ip_arg(argv[2], argv[3]) == -1) {
		fprintf(stderr, "IP Argument Error\n");
		exit(1);
	}

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

